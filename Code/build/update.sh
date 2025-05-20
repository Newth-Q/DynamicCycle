#!/bin/bash
# 严格模式：避免未定义变量和错误自动退出
set -m

# 配置部分
readonly CSV_FILE="para.csv"         # 参数文件路径
readonly EXECUTABLE="./DynamicCycle" # 可执行文件路径
readonly MAX_RUNTIME=21600           # 6小时超时（单位：秒）
readonly REPEAT_TIMES=3             # 重复实验次数
ALGORITHMS=(3 4 5)                    # 算法类型数组

# 颜色定义用于输出高亮
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

trap 'echo "Stopping..."; kill 0; exit 1' SIGINT

# 主逻辑：逐行处理CSV文件
while IFS=, read -r dataset points variables time_type window1 window2 window3 window4; do
    # 跳过标题行（兼容中文/英文标题）
    if [[ "$dataset" =~ ^[[:alnum:]]*数据集 ]] || [[ "$dataset" == "dataset" ]]; then
        echo -e "${YELLOW}[INFO] 跳过标题行: $dataset${NC}"
        continue
    fi

    # 基础参数校验
    if ! [[ "$points" =~ ^[0-9]+$ ]] || ! [[ "$variables" =~ ^[0-9]+$ ]]; then
        echo -e "${RED}[ERROR] 非法数字参数: 点数=$points 边数=$variables${NC}"
        exit 1
    fi

    # 遍历所有算法类型
    for algo in "${ALGORITHMS[@]}"; do
        echo -e "\n${GREEN}[PROCESS] 数据集=$dataset | 算法=$algo${NC}"
        declare -i window_counter=0   # 全局计数器

        # 遍历时间窗口 (window1~window4)
        for window in "$window1" "$window2" "$window3" "$window4"; do
            ((window_counter++))

            declare -i update_number=($variables)/10

            # 执行重复实验
            for ((repeat=1; repeat<=REPEAT_TIMES; repeat++)); do
                echo -e "[TASK] 执行实验：窗口=$window (${window_counter}/4) | 重复=$repeat/${REPEAT_TIMES}"
                
                # 使用timeout控制执行时间
                if timeout $MAX_RUNTIME $EXECUTABLE \
                    -i "$dataset" \
                    -n "$points" \
                    -m "$variables" \
                    -t "$time_type" \
                    -w "$window" \
                    -a "$algo" \
                    -u "$update_number"; then
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

echo -e "\n${GREEN}[COMPLETE] 所有实验执行完毕${NC}"