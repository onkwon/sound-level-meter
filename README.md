# 소리 크기 측정
STM32F1 와 MAX4466 로 만드는 간단한 소리 크기 측정기

[FFT 관련 내용](fft.md)을 추가했습니다.

## 사용한 모듈
* 마이크: MAX4466
  - [adafruit](https://www.adafruit.com/product/1063)
  - [aliexpress](https://www.aliexpress.com/w/wholesale-max4466.html)
* MCU: [bluepill](https://www.aliexpress.com/item/1005001833796256.html)

## 소리란
소리는 공간에 퍼지는 파동이다. 우리는 공간의 특정 지점에서 측정된 진폭으로
파동을 해석할 수 있다. 그러니까 특정 지점에 놓인 마이크는 그곳을 통과하는 파동의
진폭으로 파형waveform을 기록할 수 있다.

우리는 주어진 일정 시간 동안 유한한 측정을 할 수 있을 뿐이다. 이러한 측정을
샘플링이라 하고, 이는 연속하지 않은 discrete signal 이다.

## 아날로그 입력 처리
우리가 사용할 STM32F103 MCU는 12비트 ADC를 제공한다. ADC 레퍼런스 전압에 3.3V를
인가하고 소리의 변화를 감지하기 위해서는 최소한 3.3V/4096 = 0.805mV 의 변화가
발생해야 한다.

마이크가 작은 소리에 이정도의 전압을 제공하지 못할 경우 증폭기를 사용한다. 반면,
게인이 높은 앰프를 사용한 마이크가 큰 소리에 레퍼런스 전압 이상의 전압을 제공할
경우 샘플링 값은 뭉게진다clipping. 따라서 상황에 맞게 앰프의 게인을 맞춰야 한다.

### STM32F103 의 ADC
ADC 클럭은 14MHz를 초과하면 안된다. RCC의 ADC 프리스케일로 14MHz를 초과하지
않도록 클럭을 설정한다. 설정 가능한 샘플타임과 더불어 12.5 클럭 사이클이 추가로
소요된다. 예컨대, 239.5 샘플타임을 설정할 경우 샘플링에 239.5 + 12.5 = 252 클럭
사이클이 소요된다. 이때 ADC 클럭이 14MHz라고 한다면, 최대 샘플링 속도는
252/14 = 18us, 1/18 ~= 56KHz가 된다.

예제에서는 4MHz ADC 클럭과 239.5 샘플타임을 사용한다. 따라서 최대 샘플링 속도는
252/4 = 63us, 1/63 ~= 16KHz가 된다.

나이퀴스트 정리에 따르면, 측정하려는 신호의 최대 주파수 2배에 해당하는 빈도로
샘플링하면 원래의 신호를 완전하게 기록할 수 있다(양자화 오차는 무시).

## 소리 크기 계산
샘플링한 최대값을 바로 사용할 수 있지만, outlier 에 크게 영향받기 때문에 샘플
전체를 통해 대표값을 산출하는 RMS를 사용한다.

그리고 소리크기를 나타낼 단위로 데시벨[^1]을 사용한다. 데시벨은 상대값이기
때문에 기준을 정해야 한다. 우리는 임의로 ADC 해상도의 중간값(4096/2=2048)으로
잡는다. 이 중간값은 결국 측정 최대값이 된다.

```c
uint32_t avg = 0;
uint32_t max = 0;
uint64_t square_sum = 0;
uint32_t amp_base = 4096/2;

for (unsigned int i = 0; i < samples; i++) {
	while (!adc_is_completed(PERIPH_ADC1)) { /* waiting */ }
	uint32_t adc = adc_get_measurement(PERIPH_ADC1);
	uint32_t amp = abs(adc - amp_base);
	avg += amp;
	max = MAX(amp, max);
	square_sum += amp * amp;
}

avg /= samples;
float rms = sqrtf(square_sum / samples);
float dB = 20.f * log10f(rms / amp_base);
```

## 참고
[^1]: 벨은 분수와 같이 기준값에 대한 현재값의 비율을 나타낸다. 다만 분수와 달리 상용로그를 취한 뒤 그 비율을 나타낸다: ![B=\log_{10}\frac{P}{P_0}](https://latex.codecogs.com/svg.latex?B=\log_{10}\frac{P}{P_0}) 데시벨은 벨에 데시deci를 붙인 것으로 양변에 10^-1 한 것과 같다: ![dB=10\log_{10}\frac{P}{P_0}](https://latex.codecogs.com/svg.latex?dB=10\log_{10}\frac{P}{P_0}) 벨보다 작은 단위를 다루기 위해 데시벨을 사용한다.

* [https://blog.yavilevich.com/2016/08/arduino-sound-level-meter-and-spectrum-analyzer](https://blog.yavilevich.com/2016/08/arduino-sound-level-meter-and-spectrum-analyzer)
* [https://ghebook.blogspot.com/2014/06/decibel-and-logarithmic-function.html](https://ghebook.blogspot.com/2014/06/decibel-and-logarithmic-function.html)
