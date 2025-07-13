#!/bin/bash
echo "Дата: $(date)"

echo -e "\nСвободное место на диске:"
df -h

echo -e "\nИспользование оперативной памяти:"
free -h
