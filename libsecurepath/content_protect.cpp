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

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "tlsecdrm_api.h"
#define LOG_TAG "drm_content_protect"
#include "log.h"
#include "tlc_communication.h"
#include "content_protect.h"

mc_comm_ctx cp_ctx;

struct protect_info {
	uint32_t dev;
	uint32_t enable;
};

#define SMEM_PATH	"/dev/s5p-smem"

#define SECMEM_IOC_SET_TZPC	_IOWR('S', 11, struct protect_info)

#define PROTECT_DEV_MFC0	0
#define PROTECT_DEV_MFC1	1
#define PROTECT_DEV_HEVC	2
#define PROTECT_DEV_GSC0	3
#define PROTECT_DEV_GSC1	4
#define PROTECT_DEV_GSC2	5

extern "C" cpResult_t CP_Enable_Path_Protection(uint32_t protect_ip)
{
	cpResult_t cp_result = CP_SUCCESS;
	struct protect_info prot;
	int fd_secmem, ret;

	LOG_I("[CONTENT_PROTECT] : CP_Enable_Path_Protection");

	fd_secmem = open(SMEM_PATH, O_RDWR);
	if (fd_secmem < 0) {
		LOG_E("s5p-smem open error!!");
		return CP_ERROR_ENABLE_PATH_PROTECTION_FAILED;
	}

	switch (protect_ip) {
	case CP_PROTECT_MFC:
		prot.dev = PROTECT_DEV_MFC0;
		break;
	case CP_PROTECT_MFC1:
		prot.dev = PROTECT_DEV_MFC1;
		break;
	case CP_PROTECT_GSC0:
		prot.dev = PROTECT_DEV_GSC0;
		break;
	case CP_PROTECT_GSC1:
		prot.dev = PROTECT_DEV_GSC1;
		break;
	case CP_PROTECT_GSC2:
		prot.dev = PROTECT_DEV_GSC2;
		break;
	default:
		prot.dev = -1;
		LOG_E("Fail to protect Content path due to wrong ID (%d)", protect_ip);
		close(fd_secmem);
		return CP_ERROR_ENABLE_PATH_PROTECTION_FAILED;
	}

	prot.enable = 1;
	ret = ioctl(fd_secmem, SECMEM_IOC_SET_TZPC, &prot);
	if (ret != 0) {
		LOG_E("Fail to get SECMEM SET TZPC:SET TZPC ret(%d)", ret);
		close(fd_secmem);
		return CP_ERROR_ENABLE_PATH_PROTECTION_FAILED;
	}
	close(fd_secmem);

	LOG_I("[CONTENT_PROTECT] : CP_Enable_Path_Protection. return value(%d)", cp_result);
	return cp_result;
}

extern "C" cpResult_t CP_Disable_Path_Protection(uint32_t protect_ip)
{
	cpResult_t cp_result = CP_SUCCESS;
	struct protect_info prot;
	int fd_secmem, ret;

	LOG_I("[CONTENT_PROTECT] : CP_Disable_Path_Protection");

	fd_secmem = open(SMEM_PATH, O_RDWR);
	if (fd_secmem < 0) {
		LOG_E("s5p-smem open error!!");
		return CP_ERROR_DISABLE_PATH_PROTECTION_FAILED;
	}

	switch (protect_ip) {
	case CP_PROTECT_MFC:
		prot.dev = PROTECT_DEV_MFC0;
		break;
	case CP_PROTECT_MFC1:
		prot.dev = PROTECT_DEV_MFC1;
		break;
	case CP_PROTECT_GSC0:
		prot.dev = PROTECT_DEV_GSC0;
		break;
	case CP_PROTECT_GSC1:
		prot.dev = PROTECT_DEV_GSC1;
		break;
	case CP_PROTECT_GSC2:
		prot.dev = PROTECT_DEV_GSC2;
		break;
	default:
		prot.dev = -1;
		LOG_E("Fail to protect Content path due to wrong ID (%d)", protect_ip);
		close(fd_secmem);
		return CP_ERROR_ENABLE_PATH_PROTECTION_FAILED;
	}

	prot.enable = 0;
	ret = ioctl(fd_secmem, SECMEM_IOC_SET_TZPC, &prot);
	if (ret != 0) {
		LOG_E("Fail to get SECMEM SET TZPC:SET TZPC ret(%d)", ret);
		close(fd_secmem);
		return CP_ERROR_DISABLE_PATH_PROTECTION_FAILED;
	}
	close(fd_secmem);

	LOG_I("[CONTENT_PROTECT] : CP_Disable_Path_Protection. return value(%d)", cp_result);
	return cp_result;
}

