/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - convert to FreeRTOS
 *******************************************************************************/

#include "HAL_Rahio.h"

#define PLATFORM_LOG_D(format, ...)                                \
    do {                                                                   \
        printf("D: %d %s() | " format "\n", __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                             \
    } while (0);

#define PLATFORM_LOG_E(format, ...)                                \
    do {                                                                   \
        printf("E: %d %s() | " format "\n", __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                             \
    } while (0);
    
uint64_t aliot_platform_time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}


uint64_t HAL_UptimeMs(void)
{
    return aos_now_ms();
}


int ThreadStart(Thread* thread, void (*fn)(void*), void* arg)
{
	int rc = 0;
   
	uint16_t usTaskStackSize = 1024*4;
	aos_task_new("mqttthread",fn,arg,usTaskStackSize);

	return rc;
}


void MutexInit(Mutex* mutex)
{
	aos_mutex_new(&mutex);
}

int MutexLock(Mutex* mutex)
{
	if (NULL != mutex) {
        aos_mutex_lock((aos_mutex_t *)&mutex->mutex, AOS_WAIT_FOREVER);
    }
}

int MutexUnlock(Mutex* mutex)
{
	if (NULL != mutex) {
        aos_mutex_unlock((aos_mutex_t *)&mutex);
    }
}


void TimerCountdownMS(Timer* timer, unsigned int timeout_ms)
{
	if (!timer) {
        return;
    }

    timer->time = HAL_UptimeMs() + timeout_ms;
}


void TimerCountdown(Timer* timer, unsigned int timeout) 
{
	TimerCountdownMS(timer, timeout * 1000);
}


int TimerLeftMS(Timer* end) 
{
	uint32_t now, res;

    if (!end) {
        return 0;
    }

    if (TimerIsExpired(end)) {
        return 0;
    }

    now = HAL_UptimeMs();
    res = end->time - now;
    return res;
}


char TimerIsExpired(Timer* timer)
{
	uint32_t cur_time;

    if (!timer) {
        return 1;
    }

    cur_time = HAL_UptimeMs();
    /*
     *  WARNING: Do NOT change the following code until you know exactly what it do!
     *
     *  check whether it reach destination time or not.
     */
    if ((cur_time - timer->time) < (UINT32_MAX / 2)) {
        return 1;
    } else {
        return 0;
    }
}


void TimerInit(Timer* timer)
{
	if (!timer) {
        return;
    }

    timer->time = 0;
}


int Rhino_read(Network* n, unsigned char* buf, int len, int timeout_ms)
{
	int            ret, err_code;
    uint32_t       len_recv;
    uint64_t       t_end, t_left;
    fd_set         sets;
    struct timeval timeout;

    t_end    = HAL_UptimeMs() + timeout_ms;
    len_recv = 0;
    err_code = 0;
    
    timeout.tv_sec  = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    ret = setsockopt(n->my_socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    if(ret < 0) {
        PLATFORM_LOG_E("setsockopt failed"); 
        return ret;
    }
    do {
        ret = recv(n->my_socket, buf + len_recv, len - len_recv, 0);
        if (ret > 0) {
            len_recv += ret;
        } else if (0 == ret) {
            PLATFORM_LOG_E("connection is closed");
            err_code = -1;
            break;
        } else {
            if (EINTR == errno) {
                PLATFORM_LOG_D("EINTR be caught");
                continue;
            }
            PLATFORM_LOG_E("read fail");
            err_code = -2;
            break;
        }

        t_left = aliot_platform_time_left(t_end, HAL_UptimeMs());
        if (0 == t_left) {
            break;
        }
    } while ((len_recv < len));

    // priority to return data bytes if any data be received from TCP
    // connection. It will get error code on next calling
    return (0 != len_recv) ? len_recv : err_code;
}


int Rhino_write(Network* n, unsigned char* buf, int len, int timeout_ms)
{
	 int      ret, err_code;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set   sets;

    t_end    = HAL_UptimeMs() + timeout_ms;
    len_sent = 0;
    err_code = 0;
    ret      = 1; // send one time if timeout_ms is value 0

    do {
        t_left = aliot_platform_time_left(t_end, HAL_UptimeMs());

        if (0 != t_left) {
            struct timeval timeout;

            FD_ZERO(&sets);
            FD_SET(n->my_socket, &sets);

            timeout.tv_sec  = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;
            ret = select(n->my_socket + 1, NULL, &sets, NULL, &timeout);
            if (ret > 0) {
                if (0 == FD_ISSET(n->my_socket, &sets)) {
                    PLATFORM_LOG_D("Should NOT arrive");
                    // If timeout in next loop, it will not sent any data
                    ret = 0;
                    continue;
                }
            } else if (0 == ret) {
                // PLATFORM_LOG_D("select-write timeout %lu", fd);
                break;
            } else {
                if (EINTR == errno) {
                    PLATFORM_LOG_D("EINTR be caught");
                    continue;
                }

                err_code = -1;
                PLATFORM_LOG_E("select-write fail");
                break;
            }
        }

        if (ret > 0) {
            ret = send(n->my_socket, buf + len_sent, len - len_sent, 0);
            if (ret > 0) {
                len_sent += ret;
            } else if (0 == ret) {
                PLATFORM_LOG_D("No data be sent");
            } else {
                if (EINTR == errno) {
                    PLATFORM_LOG_D("EINTR be caught");
                    continue;
                }

                err_code = -1;
                PLATFORM_LOG_E("send fail");
                break;
            }
        }
    } while ((len_sent < len) &&
             (aliot_platform_time_left(t_end, HAL_UptimeMs()) > 0));

    return err_code == 0 ? len_sent : err_code;
}


void Rhino_disconnect(Network* n)
{

    // Shutdown both send and receive operations.
     shutdown((int)n->my_socket, 2);
    
     close((int)n->my_socket);
}


void NetworkInit(Network* n)
{
	n->my_socket = 0;
	n->mqttread = Rhino_read;
	n->mqttwrite = Rhino_write;
	n->disconnect = Rhino_disconnect;
}


int NetworkConnect(Network* n, char* addr, int port)
{
    struct sockaddr_in saddr;
    int rc = -1;

    memset(&saddr, 0, sizeof(saddr));

    PLATFORM_LOG_D(
                "establish tcp connection with server(host=%s port=%u)", addr, port);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(addr);
    
    n->my_socket = socket(AF_INET,SOCK_STREAM,0);
    if (n->my_socket < 0) {
         PLATFORM_LOG_E("create socket error");
        rc = -1;
    }
    

	if ((rc = connect(n->my_socket, (struct sockaddr*)&saddr,sizeof(saddr))) < 0)
	{
		 close((int)n->my_socket);
	    goto exit;
	}

exit:
	return rc;
}


#if 0
int NetworkConnectTLS(Network *n, char* addr, int port, SlSockSecureFiles_t* certificates, unsigned char sec_method, unsigned int cipher, char server_verify)
{
	SlSockAddrIn_t sAddr;
	int addrSize;
	int retVal;
	unsigned long ipAddress;

	retVal = sl_NetAppDnsGetHostByName(addr, strlen(addr), &ipAddress, AF_INET);
	if (retVal < 0) {
		return -1;
	}

	sAddr.sin_family = AF_INET;
	sAddr.sin_port = sl_Htons((unsigned short)port);
	sAddr.sin_addr.s_addr = sl_Htonl(ipAddress);

	addrSize = sizeof(SlSockAddrIn_t);

	n->my_socket = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_SEC_SOCKET);
	if (n->my_socket < 0) {
		return -1;
	}

	SlSockSecureMethod method;
	method.secureMethod = sec_method;
	retVal = sl_SetSockOpt(n->my_socket, SL_SOL_SOCKET, SL_SO_SECMETHOD, &method, sizeof(method));
	if (retVal < 0) {
		return retVal;
	}

	SlSockSecureMask mask;
	mask.secureMask = cipher;
	retVal = sl_SetSockOpt(n->my_socket, SL_SOL_SOCKET, SL_SO_SECURE_MASK, &mask, sizeof(mask));
	if (retVal < 0) {
		return retVal;
	}

	if (certificates != NULL) {
		retVal = sl_SetSockOpt(n->my_socket, SL_SOL_SOCKET, SL_SO_SECURE_FILES, certificates->secureFiles, sizeof(SlSockSecureFiles_t));
		if (retVal < 0)
		{
			return retVal;
		}
	}

	retVal = sl_Connect(n->my_socket, (SlSockAddr_t *)&sAddr, addrSize);
	if (retVal < 0) {
		if (server_verify || retVal != -453) {
			sl_Close(n->my_socket);
			return retVal;
		}
	}

	SysTickIntRegister(SysTickIntHandler);
	SysTickPeriodSet(80000);
	SysTickEnable();

	return retVal;
}
#endif
