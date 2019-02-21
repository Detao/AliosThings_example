#include <k_api.h>
#include "audio.h"
#include "stm32l4xx_hal.h"
#include "soc_init.h"

//#define FLASH_MONO_DATA

#define PART_FOR_AUDIO            HAL_PARTITION_OTA_TEMP
#define HANDLE_SAI                hsai_BlockA2
#define HANDLE_DMA                hdma_sai2_a

typedef enum {
	SAI_dir_tx_m2p,
	SAI_dir_rx_p2m
} SAI_DIRECTION;

#define SAMPLE_RATE               8000
#define SAI_DATASIZE              16
#define SAI_DATA_BYTES            (SAI_DATASIZE / 8)
#define DATA_BUFF_LEN             ((16 << 10) / SAI_DATA_BYTES)

/* in millisecond */
#define DMA_WAIT_TIMEOUT          8000
#define SAI_WAIT_TIMEOUT          1000

#define MAX_SIZE_PATH             256
#define SDCARD_AUDIO_DIR          "/sdcard/devkit_audio"

#if (SAI_DATASIZE == 8)
typedef uint8_t SAI_datatype;
#elif (SAI_DATASIZE == 16)
typedef uint16_t SAI_datatype;
#elif (SAI_DATASIZE == 32)
typedef uint32_t SAI_datatype;
#else
#error "Invalid sai datasize!"
#endif

static SAI_datatype data_buff[DATA_BUFF_LEN] = {0};
static __IO int16_t UpdatePointer = -1;
static aos_mutex_t sai_mutex;
static aos_sem_t audio_sem;

extern SAI_HandleTypeDef HANDLE_SAI;
extern DMA_HandleTypeDef HANDLE_DMA;

static uint32_t get_audio_part_len(void)
{
	hal_logic_partition_t *audio_part = hal_flash_get_info(PART_FOR_AUDIO);

	return audio_part->partition_length;
}

static float get_run_time(void)
{
	float time_in_sec = 0.0;

#ifdef FLASH_MONO_DATA
	time_in_sec = get_audio_part_len() / (SAMPLE_RATE * SAI_DATA_BYTES);
#else
	time_in_sec = get_audio_part_len() / (SAMPLE_RATE * SAI_DATA_BYTES * 2);
#endif

	return time_in_sec;
}

static int reinit_sai_and_dma(SAI_DIRECTION dir)
{
	static SAI_DIRECTION sai_dma_status = SAI_dir_rx_p2m;

	if (dir == sai_dma_status)
		return 0;

	switch (dir) {
		case SAI_dir_tx_m2p:
			hal_gpio_output_high(&brd_gpio_table[GPIO_AUDIO_CTL]);
			break;
		case SAI_dir_rx_p2m:
			hal_gpio_output_low(&brd_gpio_table[GPIO_AUDIO_CTL]);
			break;
		default:
			KIDS_A10_PRT("Parameters is invalied, dir: %d\n", dir);
			return -1;
	}

	HANDLE_SAI.Init.AudioMode = (dir == SAI_dir_tx_m2p ? SAI_MODESLAVE_TX : SAI_MODESLAVE_RX);
	HANDLE_SAI.Init.ClockStrobing = (dir == SAI_dir_tx_m2p ? SAI_CLOCKSTROBING_FALLINGEDGE : SAI_CLOCKSTROBING_RISINGEDGE);
	if (HAL_SAI_Init(&HANDLE_SAI) != HAL_OK)
	{
		KIDS_A10_PRT("HAL_SAI_Init return failed.\n");
		return -1;
	}

	HANDLE_DMA.Init.Direction = (dir == SAI_dir_tx_m2p ? DMA_MEMORY_TO_PERIPH : DMA_PERIPH_TO_MEMORY);
	if (HAL_DMA_Init(&HANDLE_DMA) != HAL_OK)
	{
		KIDS_A10_PRT("HAL_DMA_Init return failed.\n");
		return -1;
	}
	sai_dma_status = dir;

	return 0;
}

/* stereo to mono */
static void ready_to_save(SAI_datatype *data, uint32_t data_num)
{
	uint32_t i;
	uint32_t frame_num = data_num / 2;

	if (data == NULL || data_num < 2) {
		KIDS_A10_PRT("Parameters is invalid.\n");
		return;
	}

	for (i = 1; i < frame_num; ++i)
		data[i] = data[i << 1];

	memset(&data[frame_num], 0x00, frame_num * SAI_DATA_BYTES);
}

/* mono to stereo */
static void ready_to_send(SAI_datatype *data, uint32_t data_num)
{
	uint32_t i;

	if (data == NULL || data_num < 1) {
		KIDS_A10_PRT("Parameters is invalid.\n");
		return;
	}

	for (i = data_num - 1; i > 0; --i) {
		data[i << 1] = data[i];
		data[(i << 1) + 1] = data[i << 1];
	}

	data[1] = data[0];
}

/**
* @brief This function handles DMA2 channel1 global interrupt.
*/
void DMA1_Channel6_IRQHandler(void)
{
	krhino_intrpt_enter();
	HAL_DMA_IRQHandler(&HANDLE_DMA);
	krhino_intrpt_exit();
}

/**
* @brief This function handles SAI1 global interrupt.
*/
void SAI2_IRQHandler(void)
{
	krhino_intrpt_enter();
	HAL_SAI_IRQHandler(&HANDLE_SAI);
	krhino_intrpt_exit();
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
	UpdatePointer = DATA_BUFF_LEN / 2;
	aos_sem_signal(&audio_sem);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
	UpdatePointer = 0;
	aos_sem_signal(&audio_sem);
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
	UpdatePointer = DATA_BUFF_LEN / 2;
	aos_sem_signal(&audio_sem);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
	UpdatePointer = 0;
	aos_sem_signal(&audio_sem);
}

int record_to_flash(void)
{
	int ret = 0;
	int uret = 0;
	int pos_buff = 0;
	uint32_t pos_flash = 0;
	uint32_t part_len = DATA_BUFF_LEN / 2;
	uint32_t part_bytes = part_len * SAI_DATA_BYTES;
	uint32_t total_size = get_audio_part_len();

	if (!aos_mutex_is_valid(&sai_mutex)) {
		KIDS_A10_PRT("aos_mutex_is_valid return false.\n");
		return -1;
	}
	ret = aos_mutex_lock(&sai_mutex, SAI_WAIT_TIMEOUT);
	if (ret != 0) {
		KIDS_A10_PRT("SAI is very busy now.\n");
		return -1;
	}

	if (!aos_sem_is_valid(&audio_sem)) {
		KIDS_A10_PRT("aos_sem_is_valid return false.\n");
		ret = -1;
		goto REC_EXIT;
	}
	ret = reinit_sai_and_dma(SAI_dir_rx_p2m);
	if (ret != 0) {
		ret = -1;
		goto REC_EXIT;
	}

	printf("Record time is %f seconds!\n", get_run_time());

	ret = HAL_SAI_Receive_DMA(&HANDLE_SAI, (uint8_t *)data_buff, DATA_BUFF_LEN);
	if (ret != 0) {
		KIDS_A10_PRT("HAL_SAI_Receive_DMA return failed.\n");
		ret = -1;
		goto REC_EXIT;
	}

#ifdef FLASH_MONO_DATA
	while (1) {
		/* Wait a callback event */
		while (UpdatePointer == -1) {
			ret = aos_sem_wait(&audio_sem, DMA_WAIT_TIMEOUT);
			if (ret != 0) {
				KIDS_A10_PRT("DMA timeout.\n");
				break;
			}
		}
		if (ret != 0) {
			ret = -1;
			break;
		}

		pos_buff = UpdatePointer;
		UpdatePointer = -1;

		/* Upate the first or the second part of the buffer */
		ready_to_save(&data_buff[pos_buff], part_len);
		ret = hal_flash_write(PART_FOR_AUDIO, &pos_flash, &data_buff[pos_buff], part_bytes / 2);
		if (ret != 0) {
			ret = -1;
			break;
		}

		/* check the end of the file */
		if ((pos_flash + part_bytes / 2) > total_size) {
			ret = 0;
			break;
		}

		if (UpdatePointer != -1) {
			/* Buffer update time is too long compare to the data transfer time */
			KIDS_A10_PRT("UpdatePointer error. Maybe MCU wasted too much time in saving data.\n");
			ret = -1;
			break;
		}
	}
#else
	while (1) {
		/* Wait a callback event */
		while (UpdatePointer == -1) {
			ret = aos_sem_wait(&audio_sem, DMA_WAIT_TIMEOUT);
			if (ret != 0) {
				KIDS_A10_PRT("DMA timeout.\n");
				break;
			}
		}
		if (ret != 0) {
			ret = -1;
			break;
		}

		pos_buff = UpdatePointer;
		UpdatePointer = -1;

		/* Upate the first or the second part of the buffer */
		ret = hal_flash_write(PART_FOR_AUDIO, &pos_flash, &data_buff[pos_buff], part_bytes);
		if (ret != 0) {
			ret = -1;
			break;
		}

		/* check the end of the file */
		if ((pos_flash + part_bytes) > total_size) {
			ret = 0;
			break;
		}

		if (UpdatePointer != -1) {
			/* Buffer update time is too long compare to the data transfer time */
			KIDS_A10_PRT("UpdatePointer error. Maybe MCU wasted too much time in saving data.\n");
			ret = -1;
			break;
		}
	}
#endif

	uret = HAL_SAI_DMAStop(&HANDLE_SAI);
	if (uret != 0) {
		KIDS_A10_PRT("HAL_SAI_DMAStop return failed.\n");
		ret = -1;
	}

REC_EXIT:
	uret = aos_mutex_unlock(&sai_mutex);
	if (uret != 0) {
		KIDS_A10_PRT("SAI release failed.\n");
		ret = -1;
	}

	return ret;
}

int playback_from_flash(void)
{
	int ret = 0;
	int uret = 0;
	int pos_buff = 0;
	uint32_t pos_flash = 0;
	uint32_t part_len = DATA_BUFF_LEN / 2;
	uint32_t part_bytes = part_len * SAI_DATA_BYTES;
	uint32_t total_size = get_audio_part_len();

	if (!aos_mutex_is_valid(&sai_mutex)) {
		KIDS_A10_PRT("aos_mutex_is_valid return false.\n");
		return -1;
	}
	ret = aos_mutex_lock(&sai_mutex, SAI_WAIT_TIMEOUT);
	if (ret != 0) {
		KIDS_A10_PRT("SAI is very busy now.\n");
		return -1;
	}

	if (!aos_sem_is_valid(&audio_sem)) {
		KIDS_A10_PRT("aos_sem_is_valid return false.\n");
		ret = -1;
		goto PB_EXIT;
	}
	ret = reinit_sai_and_dma(SAI_dir_tx_m2p);
	if (ret != 0) {
		ret = -1;
		goto PB_EXIT;
	}

	printf("Playback time is %f seconds!\n", get_run_time());

#ifdef FLASH_MONO_DATA
	ret = hal_flash_read(PART_FOR_AUDIO, &pos_flash, &data_buff[pos_buff], part_bytes);
	if (ret != 0) {
		ret = -1;
		goto PB_EXIT;
	}

	ready_to_send(data_buff, part_len);

	ret = HAL_SAI_Transmit_DMA(&HANDLE_SAI, (uint8_t *)data_buff, DATA_BUFF_LEN);
	if (ret != 0) {
		KIDS_A10_PRT("HAL_SAI_Transmit_DMA return failed.\n");
		ret = -1;
		goto PB_EXIT;
	}

	while (1) {
		/* Wait a callback event */
		while (UpdatePointer == -1) {
			ret = aos_sem_wait(&audio_sem, DMA_WAIT_TIMEOUT);
			if (ret != 0) {
				KIDS_A10_PRT("DMA timeout.\n");
				break;
			}
		}
		if (ret != 0) {
			ret = -1;
			break;
		}

		pos_buff = UpdatePointer;
		UpdatePointer = -1;

		/* Upate the first or the second part of the buffer */
		ret = hal_flash_read(PART_FOR_AUDIO, &pos_flash, &data_buff[pos_buff], part_bytes / 2);
		if (ret != 0) {
			ret = -1;
			break;
		}

		ready_to_send(&data_buff[pos_buff], part_len / 2);

		/* check the end of the file */
		if ((pos_flash + part_bytes / 2) > total_size) {
			ret = 0;
			break;
		}

		if (UpdatePointer != -1) {
			/* Buffer update time is too long compare to the data transfer time */
			KIDS_A10_PRT("UpdatePointer error.\n");
			ret = -1;
			break;
		}
	}
#else
	ret = hal_flash_read(PART_FOR_AUDIO, &pos_flash, &data_buff[pos_buff], DATA_BUFF_LEN * SAI_DATA_BYTES);
	if (ret != 0) {
		ret = -1;
		goto PB_EXIT;
	}

	ret = HAL_SAI_Transmit_DMA(&HANDLE_SAI, (uint8_t *)data_buff, DATA_BUFF_LEN);
	if (ret != 0) {
		KIDS_A10_PRT("HAL_SAI_Transmit_DMA return failed.\n");
		ret = -1;
		goto PB_EXIT;
	}

	while (1) {
		/* Wait a callback event */
		while (UpdatePointer == -1) {
			ret = aos_sem_wait(&audio_sem, DMA_WAIT_TIMEOUT);
			if (ret != 0) {
				KIDS_A10_PRT("DMA timeout.\n");
				break;
			}
		}
		if (ret != 0) {
			ret = -1;
			break;
		}

		pos_buff = UpdatePointer;
		UpdatePointer = -1;

		/* Upate the first or the second part of the buffer */
		ret = hal_flash_read(PART_FOR_AUDIO, &pos_flash, &data_buff[pos_buff], part_bytes);
		if (ret != 0) {
			ret = -1;
			break;
		}

		/* check the end of the file */
		if ((pos_flash + part_bytes) > total_size) {
			ret = 0;
			break;
		}

		if (UpdatePointer != -1) {
			/* Buffer update time is too long compare to the data transfer time */
			KIDS_A10_PRT("UpdatePointer error.\n");
			ret = -1;
			break;
		}
	}
#endif

	uret = HAL_SAI_DMAStop(&HANDLE_SAI);
	if (uret != 0) {
		KIDS_A10_PRT("HAL_SAI_DMAStop return failed.\n");
		ret = -1;
	}

PB_EXIT:
	uret = aos_mutex_unlock(&sai_mutex);
	if (uret != 0) {
		KIDS_A10_PRT("SAI release failed.\n");
		ret = -1;
	}

	return ret;
}

static int sdaudio_open(const char *file_in_sd, int wflag)
{
	int ret = 0;
	aos_dir_t *dp = NULL;
	int fd = 0;
	char audio_file[MAX_SIZE_PATH] = {0};

	dp = aos_opendir(SDCARD_AUDIO_DIR);
	if (dp == NULL) {
		ret = aos_mkdir(SDCARD_AUDIO_DIR);
		if (ret != 0) {
			KIDS_A10_PRT("aos_mkdir return failed. ret = %d\n", ret);
			return -1;
		}
	} else {
		ret = aos_closedir(dp);
		if (ret != 0) {
			KIDS_A10_PRT("aos_closedir return failed.\n");
			return -1;
		}
	}

	snprintf(audio_file, MAX_SIZE_PATH, SDCARD_AUDIO_DIR"/%s", file_in_sd);
	if (wflag) {
		fd = aos_open((const char*)audio_file, O_WRONLY | O_CREAT | O_TRUNC);
	} else {
		fd = aos_open((const char*)audio_file, O_RDONLY);
	}
	if (fd < 0) {
		KIDS_A10_PRT("aos_open return failed.\n");
		return -1;
	}

	return fd;
}

int record_to_sdcard(const char *file_in_sd, int *stop_flag)
{
	int ret = 0;
	int uret = 0;
	int next_stop = 0;
	int fd = -1;
	ssize_t ret_size = 0;
	int pos_buff = 0;
	uint32_t part_len = DATA_BUFF_LEN / 2;
	uint32_t part_bytes = part_len * SAI_DATA_BYTES;

	if (file_in_sd == NULL || stop_flag == NULL || *stop_flag != 0) {
		KIDS_A10_PRT("parameters is invalid.\n");
		return -1;
	}
	if (!aos_mutex_is_valid(&sai_mutex)) {
		KIDS_A10_PRT("aos_mutex_is_valid return false.\n");
		return -1;
	}
	ret = aos_mutex_lock(&sai_mutex, SAI_WAIT_TIMEOUT);
	if (ret != 0) {
		KIDS_A10_PRT("SAI is very busy now.\n");
		return -1;
	}
	if (!aos_sem_is_valid(&audio_sem)) {
		KIDS_A10_PRT("aos_sem_is_valid return false.\n");
		ret = -1;
		goto REC_EXIT;
	}
	fd = sdaudio_open(file_in_sd, 1);
	if (fd < 0) {
		ret = -1;
		goto REC_EXIT;
	}
	aos_lseek(fd, 0, SEEK_SET);
	ret = reinit_sai_and_dma(SAI_dir_rx_p2m);
	if (ret != 0) {
		ret = -1;
		goto REC_EXIT;
	}

	ret = HAL_SAI_Receive_DMA(&HANDLE_SAI, (uint8_t *)data_buff, DATA_BUFF_LEN);
	if (ret != 0) {
		KIDS_A10_PRT("HAL_SAI_Receive_DMA return failed.\n");
		ret = -1;
		goto REC_EXIT;
	}

	while (1) {
		/* Wait a callback event */
		while (UpdatePointer == -1) {
			ret = aos_sem_wait(&audio_sem, DMA_WAIT_TIMEOUT);
			if (ret != 0) {
				KIDS_A10_PRT("DMA timeout.\n");
				break;
			}
		}
		if (ret != 0) {
			ret = -1;
			break;
		}

		pos_buff = UpdatePointer;
		UpdatePointer = -1;

		/* Upate the first or the second part of the buffer */
		ret_size = aos_write(fd, &data_buff[pos_buff], part_bytes);
		if (ret_size != part_bytes) {
			KIDS_A10_PRT("aos_write return failed.\n");
			ret = -1;
			break;
		}

		if (next_stop == 1) {
			ret = 0;
			break;
		}

		if (*stop_flag != 0) {
			next_stop = 1;
		}

		if (UpdatePointer != -1) {
			/* Buffer update time is too long compare to the data transfer time */
			KIDS_A10_PRT("UpdatePointer error. Maybe MCU wasted too much time in saving data.\n");
			ret = -1;
			break;
		}
	}

	uret = HAL_SAI_DMAStop(&HANDLE_SAI);
	if (uret != 0) {
		KIDS_A10_PRT("HAL_SAI_DMAStop return failed.\n");
		ret = -1;
	}

REC_EXIT:
	if (fd >= 0) {
		uret = aos_close(fd);
		if (uret != 0) {
			KIDS_A10_PRT("aos_close return failed.\n");
			ret = -1;
		}
	}
	uret = aos_mutex_unlock(&sai_mutex);
	if (uret != 0) {
		KIDS_A10_PRT("SAI release failed.\n");
		ret = -1;
	}

	return ret;
}

int playback_from_sdcard(const char *file_in_sd)
{
	int ret = 0;
	int uret = 0;
	int fd = -1;
	int next_stop = 0;
	ssize_t ret_size = 0;
	ssize_t read_len = 0;
	int pos_buff = 0;
	uint32_t part_len = DATA_BUFF_LEN / 2;
	uint32_t part_bytes = part_len * SAI_DATA_BYTES;
	off_t file_offset = 0;
	ssize_t total_size = 0;

	if (file_in_sd == NULL) {
		KIDS_A10_PRT("parameters is invalid.\n");
		return -1;
	}
	if (!aos_mutex_is_valid(&sai_mutex)) {
		KIDS_A10_PRT("aos_mutex_is_valid return false.\n");
		return -1;
	}
	ret = aos_mutex_lock(&sai_mutex, SAI_WAIT_TIMEOUT);
	if (ret != 0) {
		KIDS_A10_PRT("SAI is very busy now.\n");
		return -1;
	}
	if (!aos_sem_is_valid(&audio_sem)) {
		KIDS_A10_PRT("aos_sem_is_valid return false.\n");
		ret = -1;
		goto PB_EXIT;
	}
	fd = sdaudio_open(file_in_sd, 0);
	if (fd < 0) {
		ret = -1;
		goto PB_EXIT;
	}
	file_offset = aos_lseek(fd, 0, SEEK_END);
	if (file_offset <= 0) {
		KIDS_A10_PRT("aos_lseek return failed.\n");
		ret = -1;
		goto PB_EXIT;
	}
	total_size = (ssize_t)file_offset;
	aos_lseek(fd, 0, SEEK_SET);
	ret = reinit_sai_and_dma(SAI_dir_tx_m2p);
	if (ret != 0) {
		ret = -1;
		goto PB_EXIT;
	}

	read_len = DATA_BUFF_LEN * SAI_DATA_BYTES;
	if (total_size < read_len) {
		KIDS_A10_PRT("total_size %ld is too small.\n", total_size);
		ret = -1;
		goto PB_EXIT;
	}
	ret_size = aos_read(fd, &data_buff[pos_buff], read_len);
	if (ret_size != read_len) {
		KIDS_A10_PRT("aos_read return failed.\n");
		ret = -1;
		goto PB_EXIT;
	}
	total_size -= read_len;

	ret = HAL_SAI_Transmit_DMA(&HANDLE_SAI, (uint8_t *)data_buff, DATA_BUFF_LEN);
	if (ret != 0) {
		KIDS_A10_PRT("HAL_SAI_Transmit_DMA return failed.\n");
		ret = -1;
		goto PB_EXIT;
	}

	while (1) {
		/* Wait a callback event */
		while (UpdatePointer == -1) {
			ret = aos_sem_wait(&audio_sem, DMA_WAIT_TIMEOUT);
			if (ret != 0) {
				KIDS_A10_PRT("DMA timeout.\n");
				break;
			}
		}
		if (ret != 0) {
			ret = -1;
			break;
		}
		
		if (next_stop) {
			ret = 0;
			break;
		}

		pos_buff = UpdatePointer;
		UpdatePointer = -1;

		if (total_size < part_bytes) {
			read_len = total_size;
			memset(&data_buff[pos_buff] + read_len, 0x00, part_bytes - read_len);
		} else {
			read_len = part_bytes;
		}

		/* Upate the first or the second part of the buffer */
		ret_size = aos_read(fd, &data_buff[pos_buff], read_len);
		if (ret_size != read_len) {
			KIDS_A10_PRT("aos_read return failed.\n");
			ret = -1;
			break;
		}
		total_size -= read_len;

		if (total_size == 0) {
			next_stop = 1;
		}

		if (UpdatePointer != -1) {
			/* Buffer update time is too long compare to the data transfer time */
			KIDS_A10_PRT("UpdatePointer error.\n");
			ret = -1;
			break;
		}
	}

	uret = HAL_SAI_DMAStop(&HANDLE_SAI);
	if (uret != 0) {
		KIDS_A10_PRT("HAL_SAI_DMAStop return failed.\n");
		ret = -1;
	}

PB_EXIT:
	if (fd >= 0) {
		uret = aos_close(fd);
		if (uret != 0) {
			KIDS_A10_PRT("aos_close return failed.\n");
			ret = -1;
		}
	}
	uret = aos_mutex_unlock(&sai_mutex);
	if (uret != 0) {
		KIDS_A10_PRT("SAI release failed.\n");
		ret = -1;
	}

	return ret;
}

int audio_init(void)
{
	int ret = 0;

	if (aos_mutex_is_valid(&sai_mutex) || aos_sem_is_valid(&audio_sem)) {
		KIDS_A10_PRT("Audio module initialization had completed before now.\n");
		return -1;
	}

	ret = aos_mutex_new(&sai_mutex);
	if (ret != 0) {
		KIDS_A10_PRT("aos_mutex_new return failed.\n");
		return -1;
	}
	ret = aos_sem_new(&audio_sem, 0);
	if (ret != 0) {
		KIDS_A10_PRT("aos_sem_new return failed.\n");
		return -1;
	}

	return 0;
}
