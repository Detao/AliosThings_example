/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/ethernetif.c
  * @author  MCD Application Team
  * @brief   This file implements Ethernet network interface drivers for lwIP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright ? 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "aos/aos.h"
#include <k_api.h>
#include "aos/network.h"
#include <lwip/tcpip.h>
#include "netif/etharp.h"

#include "ethernetif.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* The time to block waiting for input. */
#define TIME_WAITING_FOR_INPUT                 ( AOS_WAIT_FOREVER )
/* Stack size of the interface thread */
#define INTERFACE_THREAD_STACK_SIZE            ( 1024 )
#define INTERFACE_THREAD__PRIO                 ( 12 )


/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

typedef struct {
    ip4_addr_t ip;
    ip4_addr_t netmask;
    ip4_addr_t gw;
} tcpip_ip_info_t;



/* Semaphore to signal incoming packets */
aos_sem_t    s_xSemaphore;
aos_mutex_t  s_xMutex;

struct netif lwip_netif;

tcpip_ip_info_t eth_ip_info = {0};


/* Private function prototypes -----------------------------------------------*/
static void ethernetif_input( void const * argument );



/*******************************************************************************
                       LL Driver Interface ( LwIP stack --> ETH) 
*******************************************************************************/
/**
  * @brief In this function, the hardware should be initialized.
  * Called from ethernetif_init().
  *
  * @param netif the already initialized lwip network interface structure
  *        for this ethernetif
  */
static void low_level_init(struct netif *netif)
{
  uint8_t macaddress[6]= { 0x54, 0xEE, 0x75, 0x91, 0x73, 0x7E };
  
 
  /* set netif MAC hardware address length */
  netif->hwaddr_len = ETH_HWADDR_LEN;

  /* set netif MAC hardware address */
  netif->hwaddr[0] =  macaddress[0];
  netif->hwaddr[1] =  macaddress[1];
  netif->hwaddr[2] =  macaddress[2];
  netif->hwaddr[3] =  macaddress[3];
  netif->hwaddr[4] =  macaddress[4];
  netif->hwaddr[5] =  macaddress[5];

  /* set netif maximum transfer unit */
  netif->mtu = 1500;

  /* Accept broadcast address and ARP traffic */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

  aos_sem_new(&s_xSemaphore, 0);
  aos_mutex_new(&s_xMutex);
  if(DM9000_Init()){

    printf(" DM9000_init faild /n");
  } 
  /* create the task that handles the ETH_MAC */
  sys_thread_new("netif", ethernetif_input, &lwip_netif, INTERFACE_THREAD_STACK_SIZE, INTERFACE_THREAD__PRIO);
}


/**
  * @brief This function should do the actual transmission of the packet. The packet is
  * contained in the pbuf that is passed to the function. This pbuf
  * might be chained.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
  * @return ERR_OK if the packet could be sent
  *         an err_t value if the packet couldn't be sent
  *
  * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
  *       strange results. You might consider waiting for space in the DMA queue
  *       to become available since the stack doesn't retry to send a packet
  *       dropped because of memory failure (except for the TCP timers).
  */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  DM9000_SendPacket(p);
	return ERR_OK;
}

/**
  * @brief Should allocate a pbuf and transfer the bytes of the incoming
  * packet from the interface into the pbuf.
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return a pbuf filled with the received packet (including MAC header)
  *         NULL on memory error
  */
struct pbuf * low_level_input(struct netif *netif)
{
  struct pbuf *p;
	p=DM9000_Receive_Packet();
	return p;
}

/**
  * @brief This function is the ethernetif_input task, it is processed when a packet 
  * is ready to be read from the interface. It uses the function low_level_input() 
  * that should handle the actual reception of bytes from the network
  * interface. Then the type of the received packet is determined and
  * the appropriate input function is called.
  *
  * @param netif the lwip network interface structure for this ethernetif
  */
void ethernetif_input( void const * argument )
{
  struct pbuf *p;
  struct netif *netif = (struct netif *) argument;
  
  for( ;; )
  {
    if (aos_sem_wait( &s_xSemaphore, TIME_WAITING_FOR_INPUT) == 0)
    {
      do
      {
        p = low_level_input( netif );
        if (p != NULL)
        {
          if (netif->input( p, netif) != ERR_OK )
          {
            pbuf_free(p);
          }
        }
      }while(p!=NULL);
    }
  }
}

/**
  * @brief Should be called at the beginning of the program to set up the
  * network interface. It calls the function low_level_init() to do the
  * actual setup of the hardware.
  *
  * This function should be passed as a parameter to netif_add().
  *
  * @param netif the lwip network interface structure for this ethernetif
  * @return ERR_OK if the loopif is initialized
  *         ERR_MEM if private data couldn't be allocated
  *         any other err_t on error
  */
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;

  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

static void tcpip_dhcpc_cb(struct netif *pstnetif)
{
    long long ts = aos_now();
    srand((unsigned int)ts);
    tcp_init();
    udp_init();

    if (!ip4_addr_cmp(ip_2_ip4(&pstnetif->ip_addr), IP4_ADDR_ANY4)) {
        //check whether IP is changed
        if (!ip4_addr_cmp(ip_2_ip4(&pstnetif->ip_addr), &eth_ip_info.ip) ||
                !ip4_addr_cmp(ip_2_ip4(&pstnetif->netmask), &eth_ip_info.netmask) ||
                !ip4_addr_cmp(ip_2_ip4(&pstnetif->gw), &eth_ip_info.gw)) {
            ip4_addr_set(&eth_ip_info.ip, ip_2_ip4(&pstnetif->ip_addr));
            ip4_addr_set(&eth_ip_info.netmask, ip_2_ip4(&pstnetif->netmask));
            ip4_addr_set(&eth_ip_info.gw, ip_2_ip4(&pstnetif->gw));
            printf("post got ip event ,ip is 0x%x \r\n", eth_ip_info.ip.addr);
            aos_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP, 0xdeaddead);
        }
    }
    return;
}


err_t tcpip_dhcpc_start(struct netif *pstnetif)
{
    if (NULL == pstnetif){
        LOG("%s input netif is NULL \r\n");
        return -1;
    }

    if (netif_is_up(pstnetif)) {
        if (dhcp_start(pstnetif) != ERR_OK) {
            LOG("dhcp client start failed");
            return -1;
        }
    }

    netif_set_status_callback(pstnetif, tcpip_dhcpc_cb);
}

static void tcpip_init_done(void *arg)
{
#if LWIP_IPV4
    ip4_addr_t ipaddr, netmask, gw;
    memset(&ipaddr, 0, sizeof(ipaddr));
    memset(&netmask, 0, sizeof(netmask));
    memset(&gw, 0, sizeof(gw));
    netif_add(&lwip_netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);
#endif

#if LWIP_IPV6
#if !LWIP_IPV4
    netif_add(&lwip_netif, NULL, ethernetif_init, tcpip_input);
#endif
    netif_create_ip6_linklocal_address(&lwip_netif, 1);
    lwip_netif.ip6_autoconfig_enabled = 1;
#endif

    netif_set_default(&lwip_netif);
    netif_set_up(&lwip_netif);
    tcpip_dhcpc_start(&lwip_netif);
}

int lwip_tcpip_init(void)
{
    tcpip_init(NULL, NULL);
    tcpip_init_done(NULL);
    LOG("TCP/IP initialized.");

    return 0;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
