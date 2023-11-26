check_container() {
    echo "binaryho/heat-build-env 컨테이너를 확인합니다."
    docker ps -a | grep heat-build-env > /dev/null
    if [ $? -eq 0 ]; then
        echo "binaryho/heat-build-env 컨테이너가 존재합니다."
        return 0
    else
        echo "binaryho/heat-build-env 컨테이너가 존재하지 않습니다."
        return 1
    fi
}

check_image() {
    echo "binaryho/heat-build-env 이미지를 확인합니다."
    docker images | grep binaryho/heat-build-env > /dev/null
    if [ $? -eq 0 ]; then
        echo "binaryho/heat-build-env 이미지가 존재합니다."
        return 0
    else
        echo "binaryho/heat-build-env 이미지가 존재하지 않습니다."
        return 1
    fi
}

build_image() {
    echo "binaryho/heat-build-env 이미지를 빌드합니다."
    echo "docker build -t binaryho/heat-build-env -f docker/build_env.Dockerfile ."
    docker build -t binaryho/heat-build-env -f docker/build_env.Dockerfile .
    if [ $? -eq 0 ]; then
        echo "binaryho/heat-build-env 이미지를 빌드하였습니다."
        return 0
    else
        echo "\e[1;31m binaryho/heat-build-env 이미지를 빌드하는데 실패하였습니다. \e[0m"
        exit 1
    fi
} 

build_container() {
    echo "binaryho/heat-build-env 컨테이너를 생성합니다."
    echo "docker run -it --name heat-build-env -v "$PWD/.:/usr/lib/heat" binaryho/heat-build-env /bin/bash"
    docker run -it --name heat-build-env -v "$PWD/.:/usr/lib/heat" binaryho/heat-build-env /bin/bash
    if [ $? -eq 0 ]; then
        docker stop heat-build-env > /dev/null
        echo "컨테이너 종료"
        echo "setup.sh 종료"
        echo "- Binary_ho"
        return 0
    else
        echo "\e[1;31m binaryho/heat-build-env 컨테이너를 생성하는데 실패하였습니다. \e[0m"
        exit 1
    fi
} 

run_container() {
    echo "binaryho/heat-build-env 컨테이너를 실행합니다."
    docker start heat-build-env > /dev/null
    if [ $? -eq 0 ]; then
        echo "binaryho/heat-build-env 컨테이너를 실행하였습니다."
        docker exec -it heat-build-env /bin/bash
        docker stop heat-build-env > /dev/null
        echo "컨테이너 종료"
        echo "setup.sh 종료"
        echo "- Binary_ho"
        return 0
    else
        echo "\e[1;31m binaryho/heat-build-env 컨테이너를 실행하는데 실패하였습니다. \e[0m"
        exit 1
    fi
} 

main() {
    check_container
    if [ $? -eq 0 ]; then
        run_container
    else
        check_image
        if [ $? -eq 0 ]; then
            build_container
        else
            build_image
            build_container
        fi
    fi
}

main