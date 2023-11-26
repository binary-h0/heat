# HEAT

**HEAT**는 POSIX 시그널을 활용한 애플리케이션 모니터링 프로그램입니다.

### **현재 구현 중**

<br/>

## 설치 및 실행 방법

---

<br/>

## 기능

---

### -i, --interval

일정 주기마다 명령어
`heat -i 30 curl -sf localhost/health_check`

### -s, --script

지정된 경로의 쉘 스크립트 실행
`heat -i 30 -s ./check`

### --pid

실패를 감지할 경우 지정된 프로세스에 시그널을 보냄
`heat -i 30 -s ./check --pid 1234 --signal USR1`

- 실행시 해당 프로세스가 없으면 에러 처리 후 종료

### --signal

실패 시 전달할 시그널 정하기, 생략하는 경우 SIGHUP 전달

### --fail

실패를 감지할 경우 지정된 스크립트 실행하기
`heat -i 30 -s ./check --fail ./failure.sh`

- 실패 스크립트 수행 중에 다음 인터벌이 도달하는 경우
  1.  다음 인터벌을 수행하지 않음
  2.  failure.sh가 실행이 종료되면 이어서 바로 실행
      - 단 여럿의 인터벌에 의해 다수 누적되더라도 한번만 실행하기
- 실패 스크립트 실행 시
  - 환경변수 HEAT_FAIL_CODE 에 exit code 전달
  - 환경변수 HEAT_FAIL_TIME 에 발생시각(UnixTime) 을 전달
  - 환경변수 HEAT_FAIL_INTERVAL 에 인터벌 주기를 전달
  - 환경변수 HEAT_FAIL_PID 에 -s나 실행한 명령의 PID 전달

### --recovery

실패가 누적될 경우 recovery 스크립트 실행
`heat -i 30 -s ./check --recovery ./recovery.sh --threshold 10 --recovery-timeout 300`

- --threshold 에서 지정한 만큼 연속으로 실패할 경우
  - 생략하는 경우 기본적으로 1로 설정
  - --recovery 에서 지정한 스크립트 실행하기
- recovery 중에는 --fail 스크립트를 실행하지 않음
- recovery 실행 후
  - -s 로 지정된 스크립트나 지정된 명령을 수행하여 상태를 확인
  - --recovery-timeout 에 지정된 시간 안에
    - 검사를 지정된 간격으로 수행
      - 복구되지 않고 시간이 지나면?
        - fail 횟수를 기존에서 누적하고, 다시 recovery를 호출
      - 검사 결과가 성공이 되면 fail 초기화 정상 모드로 진입
- recovery 스크립트 실행
  - 환경변수 HEAT_FAIL_CODE 에 exit code 전달
  - 환경변수 HEAT_FAIL_TIME 에 **최초 발생시각**(UnixTime) 을 전달
  - 환경변수 HEAT_FAIL_TIME_LAST 에 **최근 발생시각**(UnixTime) 을 전달
  - 환경변수 HEAT_FAIL_INTERVAL 에 인터벌 주기를 전달
  - 환경변수 HEAT_FAIL_PID 에 -s나 실행한 명령의 PID 전달
  - 환경변수 HEAT_FAIL_CNT 에 현재까지 연속 실패한 횟수를 전달

### --threshold

실행한 명령어 혹은 스크립트의 연속된 실패 횟수 한계를 정함

- 한계를 넘으면 --recovery 호출, --fault-signal을 보낸다.
- 지정된 pid에 보낼 수 있다. 없으면 heat 프로그램이 받음.
  `heat -i 30 -s ./check --pid 1234 --threshold 10 --fault-signal STOP --success-signal CONT`

### 프로그램 에러 및 종료

검사 스크립트 혹은 검사 명령어, 복구 명령어 및 시그널 수신 프로세스가 중단되면
즉시 에러를 출력하고 종료해야함

### 로그

heat.verbose.log

- 검사 명령어/스크립트, 복구 스크립트의 모든 출력이 기록됨
- 기존의 기록에 이어서 기록
- 형식
  - 시간(초) : 타입 : 명령/프로그램 :

```log
123123123 : INFO : ./check.sh :
> .......
> ......
123123153 : INFO : ./check.sh :
> .........
> ........
```

- 로그 타입
  - INFO : 일반 정보
  - FAIL : 실패 정보
  - ERR : 처리할 수 없는 실패/에러 정보

## 사용 예제

---
