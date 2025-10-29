좋아요 👍 — 이제 **Modbus(모드버스)**에 대해 아주 자세하고 체계적으로 설명드릴게요.
산업 자동화나 IoT(사물인터넷) 환경에서 자주 등장하는 통신 프로토콜 중 하나입니다.

---

## 🧩 1. Modbus란?

**Modbus**는 **산업용 통신 프로토콜(Industrial Communication Protocol)**로,
1979년 프랑스의 **Modicon(현재는 Schneider Electric)**에서 **PLC(Programmable Logic Controller)** 간의 통신을 위해 개발한 **개방형 프로토콜(Open Protocol)**입니다.

즉,

> “여러 산업 기기들이 서로 데이터를 주고받기 위한 표준 언어”
> 라고 보면 됩니다.

---

## 🏗️ 2. Modbus의 기본 개념

| 개념              | 설명                                             |
| --------------- | ---------------------------------------------- |
| **마스터(Master)** | 데이터를 요청하는 장치 (예: PC, SCADA, HMI 등)             |
| **슬레이브(Slave)** | 데이터를 응답하는 장치 (예: PLC, 센서, 인버터 등)               |
| **통신 방식**       | 마스터가 요청(Request)을 보내고, 슬레이브가 응답(Response)하는 구조 |
| **전송 단위**       | “프레임(Frame)”이라 불리는 데이터 패킷 단위로 통신               |

> 즉, Modbus는 항상 “**Master → Slave → Master**”의 **요청-응답(Request-Response)** 구조로 동작합니다.

---

## 🔌 3. Modbus의 주요 변형(버전)

| 명칭               | 통신 방식   | 물리 계층             | 특징                          |
| ---------------- | ------- | ----------------- | --------------------------- |
| **Modbus RTU**   | 시리얼 통신  | RS-485, RS-232    | 가장 많이 사용됨. 바이너리 프레임 사용, 효율적 |
| **Modbus ASCII** | 시리얼 통신  | RS-485, RS-232    | 텍스트 기반, 디버깅 용이하지만 속도 느림     |
| **Modbus TCP**   | 이더넷     | Ethernet (TCP/IP) | 네트워크 기반, 빠르고 확장성 높음         |
| **Modbus Plus**  | 전용 네트워크 | 전용 케이블            | 폐쇄형, 속도 빠르지만 비용 높음 (거의 안 씀) |

---

## ⚙️ 4. Modbus RTU 프레임 구조 (대표적인 형태)

**RTU 프레임(요청/응답)의 일반 구조**

| 필드                        | 크기     | 설명                 |
| ------------------------- | ------ | ------------------ |
| **주소 (Address)**          | 1 Byte | 슬레이브 장치 번호 (1~247) |
| **기능 코드 (Function Code)** | 1 Byte | 어떤 동작을 수행할지 지정     |
| **데이터 (Data)**            | N Byte | 실제 전송되는 데이터        |
| **CRC (Checksum)**        | 2 Byte | 오류 검출용 (CRC-16)    |

예를 들어,

* 마스터가 슬레이브 #1에게 코일 상태를 읽으라고 요청할 수 있습니다.
* 슬레이브는 그 요청에 따라 해당 데이터를 응답합니다.

---

## 📜 5. 주요 기능 코드 (Function Code)

| 기능 코드         | 의미                       | 동작 설명               |
| ------------- | ------------------------ | ------------------- |
| **01**        | Read Coils               | 디지털 출력(코일) 읽기       |
| **02**        | Read Discrete Inputs     | 디지털 입력 읽기           |
| **03**        | Read Holding Registers   | 아날로그 출력(보유 레지스터) 읽기 |
| **04**        | Read Input Registers     | 아날로그 입력 읽기          |
| **05**        | Write Single Coil        | 단일 코일 쓰기            |
| **06**        | Write Single Register    | 단일 레지스터 쓰기          |
| **15 (0x0F)** | Write Multiple Coils     | 여러 코일 쓰기            |
| **16 (0x10)** | Write Multiple Registers | 여러 레지스터 쓰기          |

> 대부분의 장비는 **03(Read Holding Registers)**와 **06(Write Single Register)**를 기본적으로 지원합니다.

---

## 📦 6. 데이터 구조

Modbus에서 데이터를 저장하는 영역은 **레지스터(Register)**라고 부릅니다.
기본적으로 4개의 데이터 영역이 있으며, 각각 번호와 접근 방식이 다릅니다.

| 영역                            | 주소 범위       | 접근         | 데이터 크기 | 용도               |
| ----------------------------- | ----------- | ---------- | ------ | ---------------- |
| **Coils (0xxxx)**             | 00001–09999 | Read/Write | 1 bit  | 디지털 출력           |
| **Discrete Inputs (1xxxx)**   | 10001–19999 | Read Only  | 1 bit  | 디지털 입력           |
| **Input Registers (3xxxx)**   | 30001–39999 | Read Only  | 16 bit | 아날로그 입력값         |
| **Holding Registers (4xxxx)** | 40001–49999 | Read/Write | 16 bit | 아날로그 출력값, 설정 값 등 |

예:
`40001` 주소의 값은 아날로그 출력용 레지스터의 첫 번째 값입니다.

---

## 🌐 7. Modbus TCP 프레임 구조

이더넷 기반에서는 CRC가 필요 없고, 대신 TCP/IP 헤더가 추가됩니다.

| 필드                       | 크기      | 설명            |
| ------------------------ | ------- | ------------- |
| **Transaction ID**       | 2 Bytes | 요청-응답 매칭용     |
| **Protocol ID**          | 2 Bytes | 항상 0x0000     |
| **Length**               | 2 Bytes | 메시지 길이        |
| **Unit ID**              | 1 Byte  | 슬레이브 ID       |
| **Function Code + Data** | N Bytes | 실제 Modbus 데이터 |

> TCP 포트 번호는 기본적으로 **502번**을 사용합니다.

---

## 🧠 8. Modbus의 장점과 단점

| 장점             | 단점                    |
| -------------- | --------------------- |
| 단순하고 구현 쉬움     | 속도 느림 (특히 RTU/RS-485) |
| 산업 표준, 오픈 프로토콜 | 실시간성 부족 (Polling 구조)  |
| 대부분 장비에서 지원    | 보안 취약 (암호화 X)         |
| 안정적인 통신 구조     | 주소/데이터 매핑 복잡          |

---

## ⚡ 9. 실제 사용 예시

예를 들어, PLC와 온도 센서가 RS-485로 연결되어 있다고 합시다.

* **마스터 (PLC)**: “센서 1번의 온도값(주소 40001)을 읽어와라”
* **슬레이브 (센서)**: “현재 값은 23.7°C (0x0017)”라고 응답

SCADA나 HMI에서는 이 데이터를 실시간으로 표시하거나, 로깅할 수 있습니다.

---

## 🔧 10. Python 예시 (pymodbus 라이브러리 사용)

```python
from pymodbus.client import ModbusSerialClient

# Modbus RTU 클라이언트 설정
client = ModbusSerialClient(
    port='/dev/ttyUSB0',  # 시리얼 포트
    baudrate=9600,
    parity='N',
    stopbits=1,
    bytesize=8
)

client.connect()

# 슬레이브 1번의 홀딩 레지스터(40001) 읽기
response = client.read_holding_registers(address=0, count=1, unit=1)
print(response.registers)

client.close()
```

---

## 📚 요약

| 항목           | 설명                       |
| ------------ | ------------------------ |
| **프로토콜 종류**  | 산업용 통신 (Master–Slave)    |
| **주요 버전**    | RTU, ASCII, TCP          |
| **전송 매체**    | RS-232, RS-485, Ethernet |
| **기본 포트**    | TCP: 502                 |
| **데이터 단위**   | 비트 또는 16비트 레지스터          |
| **에러 검출**    | RTU: CRC16               |
| **대표 기능 코드** | 01~06, 15, 16            |

---

원하신다면, 다음과 같은 추가 자료도 만들어드릴 수 있어요:

* 📊 **Modbus RTU vs TCP 비교표**
* 🧠 **PLC/HMI와 Modbus 연결 구성도**
* 🧰 **Modbus 통신 디버깅 방법**
* 💻 **Python 또는 Node.js로 Modbus 제어 예제**

---

어떤 방향으로 더 자세히 알고 싶으신가요?
예를 들어

* “Modbus TCP를 실제 네트워크에서 설정하는 법”
* “Modbus RTU 신호선 연결과 통신 설정”
* “Modbus 패킷 구조 분석”
  중 어떤 걸 더 깊이 설명해드릴까요?
