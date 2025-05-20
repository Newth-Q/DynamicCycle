#!/bin/bash
set -m

# 配置部分
readonly CSV_FILE="para.csv"         # 参数文件路径
readonly EXECUTABLE="./DynamicCycle" # 可执行文件路径
readonly MAX_RUNTIME=21600           # 6小时超时（单位：秒）
readonly REPEAT_TIMES=3             # 重复实验次数
ALGORITHMS=(1 2)                    # 算法类型数组

# 颜色定义用于输出高亮
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

trap 'echo "Stopping..."; kill 0; exit 1' SIGINT

# 新增错误处理函数
file_check() {
    [[ -f "$CSV_FILE" ]] || { echo -e "${RED}[ERROR] 参数文件 $CSV_FILE 不存在${NC}"; exit 1; }
    [[ -x "$EXECUTABLE" ]] || { echo -e "${RED}[ERROR] 可执行文件 $EXECUTABLE 不存在或无权执行${NC}"; exit 1; }
}

# 执行前置检查
file_check

while IFS=, read -r dataset points variables time_type window1 window2 window3 window4; do
    if [[ "$dataset" =~ ^[[:alnum:]]*数据集 ]] || [[ "$dataset" == "dataset" ]]; then
        echo -e "${YELLOW}[INFO] 跳过标题行: $dataset${NC}"
        continue
    fi

    if ! [[ "$points" =~ ^[0-9]+$ ]] || ! [[ "$variables" =~ ^[0-9]+$ ]]; then
        echo -e "${RED}[ERROR] 非法数字参数: 点数=$points 边数=$variables${NC}"
        exit 1
    fi

    for algo in "${ALGORITHMS[@]}"; do
        echo -e "\n${GREEN}[PROCESS] 数据集=$dataset | 算法=$algo${NC}"

        declare -i skip_remaining=0   # 修改为整数声明
        declare -i window_counter=0   # 全局计数器

        for window in "$window1" "$window2" "$window3" "$window4"; do

            ((window_counter++))

            if (( skip_remaining )); then
                echo -e "${YELLOW}[SKIP] 因超时跳过窗口$window_counter ($window)${NC}"
                continue
            fi

            for ((repeat=1; repeat<=REPEAT_TIMES; repeat++)); do
                echo -e "[TASK] 窗口=${window} (${window_counter}/4) | 重复=$repeat/${REPEAT_TIMES}"
                
                if timeout $MAX_RUNTIME $EXECUTABLE \
                    -i "$dataset" \
                    -n "$points" \
                    -m "$variables" \
                    -t "$time_type" \
                    -w "$window" \
                    -a "$algo" ; then
                    echo -e "${GREEN}[SUCCESS] 实验完成${NC}"
                else
                    ret=$?
                    # 超时特殊处理
                    if [[ $ret -eq 124 ]]; then
                        echo -e "${RED}[TIMEOUT] 检测到6小时超时！${NC}"
                        break 2 # 跳出重复循环和窗口循环
                    else
                        echo -e "${RED}[ERROR] 程序异常退出，代码=$ret${NC}"
                        break 2
                    fi
                fi
            done
        done
    done
done < "$CSV_FILE"