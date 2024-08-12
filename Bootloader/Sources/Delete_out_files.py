import os
import shutil

def copy_exceptions(exceptions, destination):
    if not os.path.exists(destination):
        os.makedirs(destination)

    current_dir = os.getcwd()
    
    for item in exceptions:
        item_path = os.path.join(current_dir, item)
        if os.path.exists(item_path):
            shutil.copy(item_path, destination)
            print(f"{item} has been copied to {destination}")
        else:
            print(f"{item} does not exist in the current directory")

def delete_files_and_directories(targets, exceptions):
    current_dir = os.getcwd()
    
    for target in targets:
        if target in exceptions:
            print(f"{target} is in the exceptions list and will not be removed")
            continue
        
        target_path = os.path.join(current_dir, target)
        if os.path.isfile(target_path):
            os.remove(target_path)
            print(f"File {target} has been removed")
        elif os.path.isdir(target_path):
            shutil.rmtree(target_path)
            print(f"Directory {target} has been removed")
        else:
            print(f"{target} does not exist in the current directory")

# Пример использования:
targets_to_delete = [ 'Out\S7V30_bootloader\Build', 'Out\S7V30_bootloader\List', 'Out\S7V30_bootloader\Obj', 'Out\S7V30_bootloader\Exe', 'Out\S7V30_bootloader\BrowseInfo']
exceptions = ['Out\S7V30_bootloader\List\S7V30_bootloader.map', 'Out\S7V30_bootloader\Exe\S7V30_bootloader.hex']
destination = 'Out\S7V30_bootloader'  # укажите путь к директории для копирования

# Сначала копируем исключения
copy_exceptions(exceptions, destination)

# Затем удаляем файлы и директории, исключая файлы из списка исключений
delete_files_and_directories(targets_to_delete, exceptions)
