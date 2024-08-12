import os
import fnmatch
import re

def count_files_with_extension(directory, extension):
    count = 0
    for root, dirs, files in os.walk(directory):
        for filename in fnmatch.filter(files, f"*.{extension}"):
            count += 1
    return count




def is_code_line(line):
    # Убираем пробелы в начале и в конце строки
    line = line.strip()
    # Игнорируем пустые строки
    if not line:
        return False
    # Игнорируем строки с однострочными комментариями
    if line.startswith('//'):
        return False
    # Игнорируем строки с многострочными комментариями
    if line.startswith('/*') and line.endswith('*/'):
        return False
    # Игнорируем строки, которые содержат только многострочные комментарии
    if re.match(r'^\s*/\*.*\*/\s*$', line):
        return False
    return True

def count_code_lines_in_file(file_path):
    code_lines = 0
    inside_multiline_comment = False
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as file:
        for line in file:
            stripped_line = line.strip()
            # Обрабатываем начало и конец многострочных комментариев
            if inside_multiline_comment:
                if '*/' in stripped_line:
                    inside_multiline_comment = False
                continue
            elif stripped_line.startswith('/*'):
                inside_multiline_comment = True
                continue
            if is_code_line(stripped_line):
                code_lines += 1
    return code_lines

def count_lines_of_code(directory):
    total_lines = 0
    for root, dirs, files in os.walk(directory):
        for filename in files:
            if filename.endswith(('.c', '.h')):  # учитываем файлы с расширением .c и .h
                file_path = os.path.join(root, filename)
                total_lines += count_code_lines_in_file(file_path)
    return total_lines


directory = "."
extension = "c"  # укажите нужное расширение
file_count = count_files_with_extension(directory, extension)
print(f"Количество файлов с расширением .{extension}: {file_count}")
directory = "."
extension = "h"  # укажите нужное расширение
file_count = count_files_with_extension(directory, extension)
print(f"Количество файлов с расширением .{extension}: {file_count}")

directory = "."
lines_of_code = count_lines_of_code(directory)
print(f"Общее количество строк кода в проекте: {lines_of_code}")
