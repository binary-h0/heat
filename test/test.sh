
./heat -i 1 echo hello
./heat -i 1 -s ./src/check.sh
./heat -i 1 -s ./src/check.sh --pid 1234 --signal SIGUSR1
./heat -i 1 -s ./src/check.sh --fail ./src/fail.sh
./heat -i 1 -s ./src/check.sh --recovery ./src/recovery.sh --threshold 5 --recovery-timeout 7
./heat -i 1 -s ./src/check.sh --recovery ./src/recovery.sh --threshold 5 --recovery-timeout 3
./heat -i 1 -s ./src/check.sh --recovery ./src/recovery.sh --threshold 5
./heat -i 1 -s ./src/check.sh --fail ./src/fail.sh --recovery ./src/recovery.sh --threshold 5