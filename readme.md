# Arduino project / water, PH, TDS ������ �̿��� Relay ����
- ���� ������ ���� �� �����̸� �۵���Ű�� ���� �� ��ǥ

## H/W
- Arduino Mega 2560
- 16*2 LCD I2C module
- push button 3��
- Relay 3��

|sensor|model number|
|--|--|
|Water sensor 2��|SEN0204|
|TDS sensor|SEN0244|
|PH sensor|SEN0169|

## ���
- ��� �� LCD�� ���� ��(PH, TDS), ��ǥ ��(PH, TDS)�� ǥ��
	- ��� �� ��ư���� ��ǥ PH, TDS �� ���� ����
		- PH : 0.2���� ����, 7.0 �ʰ� �� 5.0���� �����Ǹ� ���� ���� �ݺ�
		- TDS : 0.1���� ����, 1.5 �ʰ� �� 0.5�� �����Ǹ� ���� ���� �ݺ�

- ����, PH ��, TDS �� �ڵ� ����

## �۵� ���
1. ������ ������ ���
1) ��ǥ ���� ���� ������ Relay ON
2) ��ǥ TDS �� ���ޱ��� Relay ON
3) ��ǥ PH �� ���ޱ��� Relay ON

2. ������ �������� ���� ���
- LCD�� ���� PH, TDS �� ǥ��
- �� ������ ON ���� �ð� ���� ��ư Ȱ��ȭ