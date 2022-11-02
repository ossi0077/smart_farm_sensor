# Arduino project / water, PH, TDS 센서를 이용한 Relay 제어
- 일정 수위에 도달 시 릴레이를 작동시키는 것이 주 목표

## H/W
- Arduino Mega 2560
- 16*2 LCD I2C module
- push button 3개
- Relay 3개

|sensor|model number|
|--|--|
|Water sensor 2개|SEN0204|
|TDS sensor|SEN0244|
|PH sensor|SEN0169|

## 기능
- 대기 시 LCD에 현재 농도(PH, TDS), 목표 농도(PH, TDS)를 표기
	- 대기 중 버튼으로 목표 PH, TDS 농도 설정 가능
		- PH : 0.2단위 증가, 7.0 초과 시 5.0으로 설정되며 이후 증가 반복
		- TDS : 0.1단위 증가, 1.5 초과 시 0.5로 설정되며 이후 증가 반복

- 수위, PH 농도, TDS 농도 자동 조절

## 작동 방식
1. 수위가 내려간 경우
1) 목표 수위 도달 전까지 Relay ON
2) 목표 TDS 농도 도달까지 Relay ON
3) 목표 PH 농도 도달까지 Relay ON

2. 수위가 내려가지 않은 경우
- LCD에 현재 PH, TDS 농도 표기
- 각 릴레이 ON 유지 시간 설정 버튼 활성화