/*
 * Copyright (C) 2012 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "tlsecdrm_api.h"
#define LOG_TAG "sec_g2ddrm"
#include "log.h"
#include "tlc_communication.h"
#include "sec_g2ddrm.h"

#define SMEM_PATH	"/dev/s5p-smem"
#define SECMEM_IOC_GET_FD_PHYS_ADDR _IOWR('S', 8, struct secfd_info)
#define ion_phys_addr_t unsigned long

struct secfd_info {
	int fd;
	ion_phys_addr_t phys;
};

mc_comm_ctx ctx;
int g_fd_secmem;

static mcResult_t tlc_initialize(void) {
	mcResult_t mcRet;

	memset(&ctx, 0x00, sizeof(ctx));
	ctx.device_id = MC_DEVICE_ID_DEFAULT;
	ctx.uuid = (mcUuid_t)TL_SECDRM_UUID;
	ctx.initialized = false;

	mcRet = tlc_open(&ctx);
	if (MC_DRV_OK != mcRet) {
		   LOG_E("open TL session failed!");
		   return mcRet;
	}

	ctx.initialized = true;

	return MC_DRV_OK;
}

static mcResult_t tlc_terminate(void) {
	mcResult_t mcRet;

	if (ctx.initialized == true) {
		mcRet = tlc_close(&ctx);
		if (MC_DRV_OK != mcRet) {
			   LOG_E("close TL session failed!");
			   return mcRet;
		}

		memset(&ctx, 0x00, sizeof(ctx));
		ctx.initialized = false;
	}

	return MC_DRV_OK;
}

int get_fd_phyaddr_from_kernel(struct secfd_info *secfd)
{
	int ret;

	ret = ioctl(g_fd_secmem, SECMEM_IOC_GET_FD_PHYS_ADDR, secfd);
	if (ret != 0) {
		LOG_E("Fail to get SECFD info: ret(%s)", strerror(errno));
		return -1;
	}

	LOG_I("___ION_FD_KERNEL::fd(%d), phyaddr(0x%x)", secfd->fd, secfd->phys);
	return 0;
}

g2ddrmResult_t sec_g2d_activate(enum driver_act act)
{
	int m_g2dFd;

	m_g2dFd = open(SEC_G2D_DEV_NAME, O_RDWR);
	if (m_g2dFd < 0) {
		LOG_E("%s::open(%s) fail(%s)\n", __func__, SEC_G2D_DEV_NAME, strerror(errno));
		return G2DDRM_ERROR_INIT_FAILED;
	}

	enum driver_act g2d_act = act;
	if (ioctl(m_g2dFd, FIMG2D_BITBLT_ACTIVATE, &g2d_act) < 0) {
		LOG_E("%s::ioctl(%s) %d fail(%s)\n", __func__, SEC_G2D_DEV_NAME, act, strerror(errno));
		close(m_g2dFd);
		return G2DDRM_ERROR_INIT_FAILED;
	}
	close(m_g2dFd);
	LOG_I("G2D %s is completed", act==DRV_DEACT? "DEACT":"ACT");

	return G2DDRM_SUCCESS;
}

extern "C" g2ddrmResult_t G2DDRM_Initialize(void)
{
	mcResult_t mcRet;
	g2ddrmResult_t ret = G2DDRM_SUCCESS;
	struct tciMessage_t *tci = NULL;

	LOG_I("G2DDRM_Initialize(): secure G2D driver initialization");
	do {
		if (sec_g2d_activate(DRV_DEACT) != 0) {
			ret = G2DDRM_ERROR_INIT_FAILED;
			break;
		}

		LOG_I("Open the Trustlet");

		g_fd_secmem = open(SMEM_PATH, O_RDWR);
		if (g_fd_secmem < 0) {
			LOG_E("open S5P-MEM device error");
			ret = G2DDRM_ERROR_INIT_FAILED;
			break;
		}

		mcRet = tlc_initialize();
		if (MC_DRV_OK != mcRet) {
			LOG_E("Tlc Open Error");
			ret = G2DDRM_ERROR_INIT_FAILED;
			close(g_fd_secmem);
			break;
		}

		LOG_I("Check TCI buffer");
		tci = ctx.tci_msg;
		if (NULL == tci) {
			LOG_E("TCI has not been set up properly - exiting");
			ret = G2DDRM_ERROR_INIT_FAILED;
			close(g_fd_secmem);
			break;
		}

		LOG_I("Prepare command message in TCI");
		tci->cmd.id = CMD_G2DDRM_INITIALIZE;
		mcRet = tlc_communicate(&ctx);
		if (MC_DRV_OK != mcRet) {
			LOG_E("tlc_communicate Error!");
			ret = G2DDRM_ERROR_INIT_FAILED;
			close(g_fd_secmem);
			break;
		}

		if ((RSP_ID(CMD_G2DDRM_INITIALIZE) != tci->resp.id)) {
			LOG_E("Trustlet did not send a response : %d", tci->resp.id);
			ret = G2DDRM_ERROR_INIT_FAILED;
			close(g_fd_secmem);
			break;
		}
		LOG_I("Trustlet response is completed");

		if (tci->resp.return_code != RET_TL_G2DDRM_OK) {
			LOG_E("Trustlet did not send a valid return code : %d", tci->resp.return_code);
			ret = G2DDRM_ERROR_INIT_FAILED;
			close(g_fd_secmem);
			break;
		}
		LOG_I("Check the Trustlet return code is completed");

		ret = G2DDRM_SUCCESS;
	} while (false);

	LOG_I("G2DDRM_Initialize(): secure G2D driver is initialized. ret(%d)", ret);
	return ret;
}

extern "C" g2ddrmResult_t G2DDRM_Blit(struct fimg2d_blit_raw *cmd)
{
	mcResult_t mcRet;
	g2ddrmResult_t ret = G2DDRM_SUCCESS;
	mcBulkMap_t mapInfo;
	struct tciMessage_t *tci = NULL;
	struct secfd_info secfd;

	mapInfo.sVirtualAddr = NULL;

	do {
		secfd.fd = cmd->src.addr.start;
		if (get_fd_phyaddr_from_kernel(&secfd) < 0) {
			LOG_E("fail to get src phyaddr from fd(%d)", secfd.fd);
			ret = G2DDRM_ERROR_BLIT_FAILED;
			break;
		}
		cmd->src.addr.start = secfd.phys;

		secfd.fd = cmd->dst.addr.start;
		if (get_fd_phyaddr_from_kernel(&secfd) < 0) {
			LOG_E("fail to get dst phyaddr from fd(%d)", secfd.fd);
			ret = G2DDRM_ERROR_BLIT_FAILED;
			break;
		}
		cmd->dst.addr.start = secfd.phys;

		secfd.fd = cmd->dst.plane2.start;
		if (get_fd_phyaddr_from_kernel(&secfd) < 0) {
			LOG_E("fail to get plane2 phyaddr from fd(%d)", secfd.fd);
			ret = G2DDRM_ERROR_BLIT_FAILED;
			break;
		}
		cmd->dst.plane2.start = secfd.phys;

		LOG_I("Check TCI buffer");
		tci = ctx.tci_msg;
		if (NULL == tci) {
			LOG_E("TCI has not been set up properly - exiting");
			ret = G2DDRM_ERROR_BLIT_FAILED;
			close(g_fd_secmem);
			break;
		}
		LOG_I("Prepare command message in TCI");

		tci->cmd.id = CMD_G2DDRM_BLIT;
		tci->blit.op = cmd->op;
		tci->blit.param = cmd->param;
		tci->blit.src = cmd->src;
		tci->blit.dst = cmd->dst;
		tci->blit.msk = cmd->msk;
		tci->blit.tmp = cmd->tmp;
		tci->blit.sync = cmd->sync;
		tci->blit.seq_no = cmd->seq_no;

		mcRet = tlc_communicate(&ctx);
		if (MC_DRV_OK != mcRet) {
			LOG_E("tlc_communicate Error!");
			ret = G2DDRM_ERROR_BLIT_FAILED;
			break;
		}

		if ((RSP_ID(CMD_G2DDRM_BLIT) != tci->resp.id)) {
			LOG_E("Trustlet did not send a response : %d", tci->resp.id);
			ret = G2DDRM_ERROR_BLIT_FAILED;
			break;
		}
		LOG_I("Trustlet response is completed");

		if (tci->resp.return_code != RET_TL_G2DDRM_OK) {
			LOG_E("Trustlet did not send a valid return code : %d", tci->resp.return_code);
			ret = G2DDRM_ERROR_BLIT_FAILED;
			break;
		}
		LOG_I("Check the Trustlet return code is completed");

		ret = G2DDRM_SUCCESS;
	} while (false);

	LOG_I("G2DDRM_Blit(): secure G2D driver blit is done. ret(%d)", ret);
	return ret;
}

extern "C" g2ddrmResult_t G2DDRM_Terminate(void)
{
	mcResult_t mcRet;
	g2ddrmResult_t ret = G2DDRM_SUCCESS;
	struct tciMessage_t *tci = NULL;

	LOG_I("G2DDRM_Terminate(): secure G2D driver termination");
	do {
		LOG_I("Check TCI buffer");

		tci = ctx.tci_msg;
		if (NULL == tci) {
			LOG_E("TCI has not been set up properly - exiting");
			ret = G2DDRM_ERROR_EXIT_FAILED;
			close(g_fd_secmem);
			break;
		}
		LOG_I("Prepare command message in TCI");
		tci->cmd.id = CMD_G2DDRM_TERMINATE;

		mcRet = tlc_communicate(&ctx);
		if (MC_DRV_OK != mcRet) {
			LOG_E("tlc_communicate Error!");
			ret = G2DDRM_ERROR_EXIT_FAILED;
			break;
		}

		if ((RSP_ID(CMD_G2DDRM_TERMINATE) != tci->resp.id)) {
			LOG_E("Trustlet did not send a response : %d", tci->resp.id);
			ret = G2DDRM_ERROR_EXIT_FAILED;
			break;
		}
		LOG_I("Trustlet response is completed");

		if (tci->resp.return_code != RET_TL_G2DDRM_OK) {
			LOG_E("Trustlet did not send a valid return code : %d", tci->resp.return_code);
			ret = G2DDRM_ERROR_EXIT_FAILED;
			break;
		}

		mcRet = tlc_terminate();
		if (MC_DRV_OK != mcRet) {
			LOG_E("Tlc Close Error");
			ret = G2DDRM_ERROR_EXIT_FAILED;
			break;
		}

		LOG_I("Check the Trustlet return code is completed");

		close(g_fd_secmem);
		if (sec_g2d_activate(DRV_ACT) != 0) {
			ret = G2DDRM_ERROR_EXIT_FAILED;
			break;
		}

		ret = G2DDRM_SUCCESS;
	} while (false);

	LOG_I("G2DDRM_Terminate(): secure G2D driver is terminated. ret(%d)", ret);
	return ret;
}
