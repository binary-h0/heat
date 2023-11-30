
./heat -i 1 echo hello
./heat -i 1 -s ./src/check.sh
./heat -i 1 -s ./src/check.sh --pid 1234 --signal SIGUSR1
./heat -i 1 -s ./src/check.sh --fail ./src/fail.sh
./heat -i 1 -s ./src/check.sh --recovery ./src/recovery.sh --threshold 10 --recovery-timeout 10