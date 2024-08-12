import ctypes

test_file_name = '4373A0.bin'

# Загрузка DLL
compress_lib = ctypes.CDLL('.\Compessor.dll')

# Определение типов аргументов и возвращаемого значения функции Compress_buf
compress_lib.Compress_buf.argtypes = [
    ctypes.c_void_p,       # in_buf
    ctypes.c_uint,         # in_buf_sz
    ctypes.c_void_p,       # out_buf
    ctypes.c_uint,         # out_buf_sz
    ctypes.c_ubyte         # alg
]
compress_lib.Compress_buf.restype = ctypes.c_int

# Определение типов аргументов и возвращаемого значения функции Decompress_buf
compress_lib.Decompress_buf.argtypes = [
    ctypes.c_void_p,       # in_buf
    ctypes.c_uint,         # in_buf_sz
    ctypes.c_void_p,       # out_buf
    ctypes.c_uint,         # out_buf_sz
    ctypes.c_ubyte         # alg
]
compress_lib.Decompress_buf.restype = ctypes.c_int


with open(test_file_name, 'rb') as file:
    input_data = file.read()
input_size = len(input_data)

# Создание выходного буфера
output_size = input_size * 2  # Предположим, что выходной буфер будет достаточно большим
output_data = ctypes.create_string_buffer(output_size)

# Алгоритм сжатия (предположим, 1)
algorithm = 0 # SIXPACK

# Упаковка
result = compress_lib.Compress_buf(
    ctypes.byref(ctypes.create_string_buffer(input_data)),
    input_size,
    ctypes.byref(output_data),
    output_size,
    algorithm
)

# Проверка результата
if result >= 0:
    print(f"Compression successful! Input size={input_size} Output Size={result}")
    compressed_data = output_data.raw[:result]
    with open(test_file_name + '_packed', 'wb') as file:
        file.write(compressed_data)
else:
    print("Compression failed with error code:", result)

# Создание выходного проверочного буфера
output2_size = input_size * 2  
output2_data = ctypes.create_string_buffer(output_size)

compress_size = result

# Распаковка
result = compress_lib.Decompress_buf(
    ctypes.byref(output_data),
    compress_size,
    ctypes.byref(output2_data),
    output2_size,
    algorithm
)

decompressed_data = output2_data.raw[:result]
with open(test_file_name + '_unpacked', 'wb') as file:
    file.write(decompressed_data)

print(f"Decompression successful! Input size={compress_size} Output Size={result}")

