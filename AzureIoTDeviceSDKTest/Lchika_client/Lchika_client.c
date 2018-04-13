#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothubtransportmqtt.h"
#include "iothub_client_options.h"
#include "serializer.h"

#define L_BLOCK_SIZE		1024 * 4
#define PERIPHERAL_BASE		0x3F000000
#define GPIO_BASE		PERIPHERAL_BASE + 0x00200000
#define GPFSEL0			0x00
#define GPSET0			0x1C
#define GPCLR0			0x28

static volatile unsigned int *gpio;
static const char* connectionString = "HostName=xxxxx;DeviceId=xxxxx;SharedAccessKey=xxxxx=";
static char msgText[1024];
static char propText[1024];

// Define the Model
BEGIN_NAMESPACE(LchikaModel);

DECLARE_MODEL(Button,
WITH_METHOD(LED_ON),
WITH_METHOD(LED_OFF)
);

END_NAMESPACE(LchikaModel);


static int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{	
    int result;

    /* receive the method and push that payload into serializer (from below)*/
    char* payloadZeroTerminated = (char*)malloc(size + 1);
    if (payloadZeroTerminated == 0)
    {
        printf("failed to malloc\r\n");
        *resp_size = 0;
        *response = NULL;
        result = -1;
    }
    else
    {
        (void)memcpy(payloadZeroTerminated, payload, size);
        payloadZeroTerminated[size] = '\0';

        /* execute method - userContextCallback is of type deviceModel */
        METHODRETURN_HANDLE methodResult = EXECUTE_METHOD(userContextCallback, method_name, payloadZeroTerminated);
        free(payloadZeroTerminated);

        if (methodResult == NULL)
        {
            printf("failed to EXECUTE_METHOD\r\n");
            *resp_size = 0;
            *response = NULL;
            result = -1;
        }
        else
        {
            /* get the serializer answer and push it in the networking stack */
            const METHODRETURN_DATA* data = MethodReturn_GetReturn(methodResult);
            if (data == NULL)
            {
                printf("failed to MethodReturn_GetReturn\r\n");
                *resp_size = 0;
                *response = NULL;
                result = -1;
            }
            else
            {
                result = data->statusCode;
                if (data->jsonValue == NULL)
                {
                    char* resp = "{}";
                    *resp_size = strlen(resp);
                    *response = (unsigned char*)malloc(*resp_size);
                    (void)memcpy(*response, resp, *resp_size);
                }
                else
                {
                    *resp_size = strlen(data->jsonValue);
                    *response = (unsigned char*)malloc(*resp_size);
                    (void)memcpy(*response, data->jsonValue, *resp_size);
                }
            }
            MethodReturn_Destroy(methodResult);
        }
    }
	
    return result;
}

METHODRETURN_HANDLE LED_ON(Button* device)
{
    (void)device;
    (void)printf("LED_ON Method.\r\n");
	gpio[GPSET0/4] |= 0b00000000000000000000000000000010;
    METHODRETURN_HANDLE result = MethodReturn_Create(0, "{\"Message\":\"LED_ON Method\"}");
    return result;
}

METHODRETURN_HANDLE LED_OFF(Button* device)
{
    (void)device;
    (void)printf("LED_OFF Method.\r\n");
	gpio[GPCLR0/4] |= 0b00000000000000000000000000000010;
    METHODRETURN_HANDLE result = MethodReturn_Create(0, "{\"Message\":\"LED_OFF Method\"}");
    return result;
}

void Lchika_client_run(void)
{
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
	
	// プラットフォーム初期化
	if (platform_init() != 0)
	{
        (void)printf("Failed to initialize platform.\r\n");
    }
	else
	{
		// シリアライザライブラリ初期化
        if (serializer_init(NULL) != SERIALIZER_OK)
		{
            (void)printf("Failed on serializer_init\r\n");
        }
        else
		{
			// IoTHub接続
            IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
            if (iotHubClientHandle == NULL)
			{
                (void)printf("Failed on IoTHubClient_LL_Create\r\n");
            }
			else
			{
				// モデルのインスタンスを生成
				Button* myModel = CREATE_MODEL_INSTANCE(LchikaModel, Button);
                if (myModel == NULL) 
				{
                    (void)printf("Failed on CREATE_MODEL_INSTANCE\r\n");
                }
                else 
				{
					// メソッドが呼び出されたときのコールバックを設定
					if(IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, DeviceMethodCallback, myModel) != IOTHUB_CLIENT_OK)
					{
						(void)printf("Failed on IoTHubClient_SetDeviceMethodCallback\r\n");
					}
					else
					{
						// キューにメソッド呼び出しがあればCallBack関数を呼び出す
						while(1) 
						{	
							IoTHubClient_LL_DoWork(iotHubClientHandle);
							ThreadAPI_Sleep(1);
						}
					}
					DESTROY_MODEL_INSTANCE(myModel);
                }
                IoTHubClient_LL_Destroy(iotHubClientHandle);
            }
            serializer_deinit();
        }
        platform_deinit();
    }
}

int main(void)
{
	int fd;
	void *map;
 
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < -1) {
		perror("open");
		return 1;
	}

	map = mmap(NULL, L_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);
	if(map == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	close(fd);
	
	gpio = (unsigned int *)map;

	gpio[GPFSEL0/4] |= 0b00000000000000000000000001000000;
	
    Lchika_client_run();
    return 0;
}
