# 기본 구현 1
heat -i 2 curl -sf https://www.google.com
# 기본 구현 2
heat -i 1 -s ./src/check.sh
# 옵션 1
heat -i 1 -s ./src/check.sh --pid 1234 --signal SIGUSR1
# 옵션 1 시그널 없이
heat -i 1 -s ./src/check.sh --pid 1234
# 옵션 2
heat -i 1 -s ./src/check.sh --fail ./src/fail.sh
# 옵션 3
heat -i 1 -s ./src/check.sh --recovery ./src/recovery.sh --threshold 5 --recovery-timeout 3
# 옵션 3 타임아웃 없이
heat -i 1 -s ./src/check.sh --recovery ./src/recovery.sh --threshold 5
# 옵션 2 + 옵션 3
heat -i 1 -s ./src/check.sh --fail ./src/fail.sh --recovery ./src/recovery.sh --threshold 5
# 옵션 4 --signal 없이
heat -i 1 -s ./src/check.sh --threshold 5 --fault-signal SIGUSR1 --success-signal SIGCHLD --pid 1234123412421
# 옵션 4, --signal 있이
heat -i 1 -s ./src/check.sh --threshold 5 --signal SIGALRM --fault-signal SIGUSR1 --success-signal SIGCHLD --pid 1234123412421
# 옵션 4 + 옵션 2 --signal 없이
heat -i 1 -s ./src/check.sh --fail ./src/fail.sh --threshold 5 --fault-signal SIGUSR1 --success-signal SIGCHLD --pid 1234123412421
# 옵션 4 + 옵션 2 --signal 있이
heat -i 1 -s ./src/check.sh --fail ./src/fail.sh --threshold 5 --signal SIGALRM --fault-signal SIGUSR1 --success-signal SIGCHLD --pid 1234123412421
# 옵션 4 + 옵션 2 + 옵션 3 --signal 없이, 타임아웃 없이
heat -i 1 -s ./src/check.sh --fail ./src/fail.sh --recovery ./src/recovery.sh --threshold 5 --fault-signal SIGUSR1 --success-signal SIGCHLD --pid 1234123412421
# 옵션 4 + 옵션 2 + 옵션 3 --signal 있이, 타임아웃 없이
heat -i 1 -s ./src/check.sh --fail ./src/fail.sh --recovery ./src/recovery.sh --threshold 5 --signal SIGALRM --fault-signal SIGUSR1 --success-signal SIGCHLD --pid 1234123412421
# 옵션 4 + 옵션 2 + 옵션 3 --signal 없이, 타임아웃 있이
heat -i 1 -s ./src/check.sh --fail ./src/fail.sh --recovery ./src/recovery.sh --threshold 5 --recovery-timeout 2 --fault-signal SIGUSR1 --success-signal SIGCHLD --pid 1234123412421
# 옵션 4 + 옵션 2 + 옵션 3 --signal 있이, 타임아웃 있이
heat -i 1 -s ./src/check.sh --fail ./src/fail.sh --recovery ./src/recovery.sh --threshold 5 --recovery-timeout 2 --signal SIGALRM --fault-signal SIGUSR1 --success-signal SIGCHLD --pid 1234123412421
