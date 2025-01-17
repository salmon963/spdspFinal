#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void process_duck_voice(float* samples, ma_uint32 frameCount, int channels)
{
    float pitchShiftFactor = 1.5f; // 提高音高
    for (ma_uint32 i = 0; i < frameCount * channels; ++i) {
        // 簡單的非線性失真模擬 "唐老鴨" 的鼻音效果
        samples[i] = tanh(samples[i] * pitchShiftFactor);

        // 加入簡單的振動效果 (模仿唐老鴨語氣特徵)
        samples[i] += 0.005f * sinf(2.0f * 3.14159265f * 400.0f * i / 44100.0f);
    }
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_encoder* pEncoder = (ma_encoder*)pDevice->pUserData;
    MA_ASSERT(pEncoder != NULL);

    float* inputSamples = (float*)pInput;

    // 處理音頻數據，模擬唐老鴨聲音
    process_duck_voice(inputSamples, frameCount, pEncoder->config.channels);

    // 將處理後的音頻數據寫入檔案
    ma_encoder_write_pcm_frames(pEncoder, inputSamples, frameCount, NULL);

    (void)pOutput; // 不需要處理輸出
}

int main(int argc, char** argv)
{
    ma_result result;
    ma_encoder_config encoderConfig;
    ma_encoder encoder;
    ma_device_config deviceConfig;
    ma_device device;

    if (argc < 2) {
        printf("No output file.\n");
        return -1;
    }

    while (1) {

        printf("Press Enter to start recording...\n");
        getchar();

        encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, ma_format_f32, 2, 44100);

        if (ma_encoder_init_file(argv[1], &encoderConfig, &encoder) != MA_SUCCESS) {
            printf("Failed to initialize output file.\n");
            return -1;
        }

        deviceConfig = ma_device_config_init(ma_device_type_capture);
        deviceConfig.capture.format = encoder.config.format;
        deviceConfig.capture.channels = encoder.config.channels;
        deviceConfig.sampleRate = encoder.config.sampleRate;
        deviceConfig.dataCallback = data_callback;
        deviceConfig.pUserData = &encoder;

        result = ma_device_init(NULL, &deviceConfig, &device);
        if (result != MA_SUCCESS) {
            printf("Failed to initialize capture device.\n");
            return -2;
        }

        result = ma_device_start(&device);
        if (result != MA_SUCCESS) {
            ma_device_uninit(&device);
            printf("Failed to start device.\n");
            return -3;
        }

        printf("Recording... Press Enter to stop.\n");
        getchar();

        ma_device_uninit(&device);
        ma_encoder_uninit(&encoder);
    }

    return 0;
}
