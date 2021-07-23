# FFT
샘플링 주파수는 48kHz, 256개의 샘플을 FFT 입력으로 사용할 예정이다. 그러므로
분석 가능한 최대 주파수는 48kHz/2 = 24kHz이고, 각 bin은48000/256 = 187.5Hz
대역을 가진다.

STM32F103 8MHZ 클럭에서 256개의 샘플을 FFT 돌리는데 대략 x초가 걸린다. 256개의
ADC 샘플을 구하는데 256/48kHz = 5.3ms가 걸린다. 그러니까 샘플 acquisition 시간
대비 프로세싱 시간은 x/5.3 = y% 가 된다. 즉, cpu load가 매우 낮기 때문에 overrun
가능성이 매우 낮다는 것이고, 이는 충분히 실현 가능한 솔루션임을 뜻한다.

## STM32F103 의 ADC
4MHz ADC 클럭과 71.5 샘플타임을 설정한 경우, (71.5+12.5)/4 = 20.75us 즉, 약
48.193kHz 샘플링 주파수를 얻는다.

## CMSIS DSP 의 FFT 코드 사이즈

64k 블루필 보드로 아래와 같은 코드를 테스트하려고 하자 100k 가깝게 코드 사이즈가
불어났다. CMSIS DSP 에서 제공하는 FFT 함수는 속도 최적화를 위해 내부적으로 bin
사이즈 별 룩업테이블을 유지하고 있기 때문이다. 따라서 사용할 bin 사이즈 테이블만
유지하고 사용하지 않는 bin 사이즈 테이블들을 제거하면 코드 사이즈를 대폭 줄일 수
있다.

```Makefile
$(DSP_PATH)/Source/CommonTables/arm_const_structs.c \
$(DSP_PATH)/Source/CommonTables/arm_common_tables.c \
$(DSP_PATH)/Source/TransformFunctions/arm_cfft_init_f32.c \
$(DSP_PATH)/Source/TransformFunctions/arm_cfft_f32.c \
$(DSP_PATH)/Source/TransformFunctions/arm_cfft_radix8_f32.c \
$(DSP_PATH)/Source/TransformFunctions/arm_bitreversal2.c \
$(DSP_PATH)/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c \
$(DSP_PATH)/Source/StatisticsFunctions/arm_max_f32.c \
```

```c
#define FFT_INPUT_SIZE		2048
#define FFT_SIZE		(FFT_INPUT_SIZE/2)
static float32_t fft_input[FFT_INPUT_SIZE];
static float32_t fft_output[FFT_SIZE];
arm_cfft_instance_f32 ctx;
arm_cfft_init_f32(&ctx, FFT_SIZE);
arm_cfft_f32(&ctx, fft_input, 0, 1);
arm_cmplx_mag_f32(fft_input, fft_output, FFT_SIZE);
float32_t max_value;
uint32_t index;
arm_max_f32(fft_output, FFT_SIZE, &max_value, &index);
```

예컨대, 1024 사이즈의 FFT를 수행한다면 다음 매크로로 1024 사이즈 테이블만 유지할
수 있다:

```
ARM_DSP_CONFIG_TABLES
ARM_FFT_ALLOW_TABLES
ARM_TABLE_TWIDDLECOEF_F32_1024
ARM_TABLE_BITREVIDX_FLT_1024
```

사실상 `ARM_DSP_CONFIG_TABLES` 매크로만 사용하면 컴파일러 최적화 단계에서
사용하는 테이블을 제외한 테이블은 자동으로 제거된다.

## 기타

* 샘플 갯수는 항상 2^n 이 되어야 한다.
* FFT 최대값은 샘플 수에 따라 결정된다
  - 256 샘플을 사용할 경우 최대값은 256
  - 1024 샘플을 사용할 경우 최대값은 1024
* FFT 첫번째 결과는 항상 DC 전압값이기 때문에 분석가능한 최대 주파수는 `샘플링 주파수/2 - 해상도`가 된다.

## References
* http://musicandcomputersbook.com/chapter3/03_05.php
