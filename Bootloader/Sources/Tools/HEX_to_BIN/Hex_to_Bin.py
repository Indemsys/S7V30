from intelhex import IntelHex

# Загрузка HEX файла
hex_file = IntelHex('brcm_patchram_buf.hex')

# Сохранение в бинарный файл
hex_file.tobinfile('brcm_patchram_buf.bin')