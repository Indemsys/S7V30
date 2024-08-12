def files_to_c_arrays_and_headers(file_paths, array_names, output_file, header_file, line_length=10):
    """
    Convert files to C arrays, save them in a single C file with a specific number of elements per line,
    and also create a header file with the array declarations.

    Args:
    file_paths (list of str): List of file paths to convert.
    array_names (list of str): List of array names for the C arrays.
    output_file (str): Path to the output C file.
    header_file (str): Path to the output header (.h) file.
    line_length (int): Number of array elements per line in the output C file.
    """
    header_content = []
    with open(output_file, 'w') as f, open(header_file, 'w') as h:
        h.write('#ifndef WEB_SERVER_CERTIFICATE_H\n')
        h.write('  #define WEB_SERVER_CERTIFICATE_H\n\n')
        f.write('#include   "App.h"\n')
        f.write('#include   "WEB_server_certificate.h"\n\n')
        for file_path, array_name in zip(file_paths, array_names):
            try:
                # Read the file and convert each byte to a C array element
                with open(file_path, 'rb') as file:
                    file_bytes = file.read()
                    array_size = len(file_bytes)
                    # Format the array elements
                    array_elements = (f'0x{byte:02x}' for byte in file_bytes)
                    formatted_array = ',\n'.join(
                        ', '.join(next(array_elements) for _ in range(line_length))
                        for _ in range(array_size // line_length)
                    )
                    # Handle the last line if the total number is not a multiple of line_length
                    remaining_elements = array_size % line_length
                    if remaining_elements:
                        last_line = ', '.join(next(array_elements) for _ in range(remaining_elements))
                        formatted_array += ',\n' + last_line
                    # Write the array to the output file
                    f.write(f'const uint8_t {array_name}[{array_size}] = {{\n{formatted_array}\n}};\n')
                    
                    # Write the header file content
                    header_content.append(f'extern const uint8_t {array_name}[{array_size}];')
            except IOError:
                print(f"Error reading {file_path}. Check if the file exists and is readable.")
        
        # Write all headers at once to the header file
        h.write('\n'.join(header_content) + '\n')
        h.write('\n#endif\n')

        print(f"All arrays and headers have been written to {output_file} and {header_file}")


file_paths = ['./Output_for_Server/srv_key.der', './Output_for_Server/srv.der']  # Список файлов
array_names = ['SERVER_KEY', 'SERVER_CERTIFICATE']  # Соответствующие имена массивов
output_file = './Output_src/WEB_server_certificate.c'  # Имя выходного файла
header_file = './Output_src/WEB_server_certificate.h'  # Имя файла заголовка



files_to_c_arrays_and_headers(file_paths, array_names, output_file, header_file, line_length=32)  # Задать количество элементов в строке
