#include "../osrms_API/osrms_API.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        printf("Uso: %s  <memory_file>\n", argv[0]);
        return 1;
    }

    // montar la memoria
    os_mount((char *)argv[1]);

    //resto de instrucciones

    // os_ls_processes
    os_ls_processes();

    // os_exists();
    int process_id = 91;
    char* file_name = "knowledg.jpg";

    int result = os_exists(process_id, file_name);

    if (result) {
        printf("Existe! Archivo: %s | ID: %d. \n", file_name, process_id);
    } else {
        printf("No existe el archivo.\n");
    }

    // os_ls_files
    os_ls_files(105);

    // os_frame_bitmap
    os_frame_bitmap();

    // os_tp_bitmap
    os_tp_bitmap();

    return 0;

}