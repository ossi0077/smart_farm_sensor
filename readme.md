# Arduino project / water, PH, TDS 센서를 이용한 Relay 제어
- 일정 수위에 도달 시 릴레이를 작동시키는 것이 주 목표

## H/W
- Arduino Mega 2560
- 16*2 LCD I2C module
- push button 3개
- Relay 3개

|sensor|model number|
|--|--|
|Water sensor|SEN0204|
|TDS sensor|SEN0244|
|PH sensor|SEN0169|

## 기능
1. 대기 시 LCD에 PH, TDS 농도를 표기
2. 버튼으로 각각의 릴레이 on 되는 시간을 설정

## 작동 방식
1. 수위가 내려간 경우
-> water relay, PH relay, TDS relay 순차적으로 설정된 시간만큼 ON
2. 수위가 내려가지 않은 경우
- LCD에 현재 PH, TDS 농도 표기
- 각 릴레이 ON 유지 시간 설정 버튼 활성화