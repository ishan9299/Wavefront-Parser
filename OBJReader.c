#include <windows.h>
#include <assert.h>

enum OBJ_KEYWORDS {
	GEOMETRIC_VERTICES,
	TEXTURE_VERTICES,
	VERTEX_NORMAL,
	FACE,
};

typedef enum OBJ_KEYWORDS OBJ_KEYWORDS;

struct memory {
    void *buffer;
    size_t size;
    size_t index;
};

typedef struct memory memory;

memory obj_vertex_geometric;
memory obj_vertex_norm;
memory obj_vertex_tex;
memory obj_face;

struct file {
    char *buffer;
    size_t size;
};

typedef struct file file;

HANDLE read_file_into_buffer(const char *file_path, file *f) {
    
    HANDLE file_handle =
        CreateFileA(file_path, GENERIC_READ, 0, 0,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
	if (file_handle != 0) {
        unsigned int file_size = 0;
        f->size = GetFileSize(file_handle, 0);
        
        char *t = (char *)VirtualAlloc(0, f->size, 
                                       MEM_COMMIT | MEM_RESERVE,
                                       PAGE_READWRITE);
        f->buffer = t;
        
        unsigned int file_bytes_read = 0;
        int read_file_flag = ReadFile(file_handle, (void *)f->buffer,
                                      f->size, &file_bytes_read, 0);
		assert(file_bytes_read == f->size);
    }
    
    return file_handle;
};


size_t read_values_into_buffer(char *file_buffer, size_t file_buffer_index,
                               memory *value_buffer, OBJ_KEYWORDS keyword)
{
	switch(keyword) {
		case GEOMETRIC_VERTICES:
		case TEXTURE_VERTICES:
		case VERTEX_NORMAL:
        {
            int *val_buffer = (int *)value_buffer->buffer;
            
            file_buffer_index += 2;
            while (file_buffer[file_buffer_index] == ' ') {
                file_buffer_index++;
            }
            float num = 0.0f;
            
			int nums_read = 0;
            int is_negative = 0;
            int found_decimal_pt = 0;
            int dec_multiplier = 10;
            float frac_multiplier = 0.1f;
            
            while(file_buffer[file_buffer_index] != 13 && file_buffer[file_buffer_index] != 10) {
                if (file_buffer[file_buffer_index] == ' ') {
                    if (is_negative == 1) {
                        num *= -1;
                    }
                    val_buffer[value_buffer->index] = *(int *)&num;
                    num = 0.0f;
                    
                    file_buffer_index++;
                    value_buffer->index++;
                    
					nums_read++;
                    
                    is_negative = 0;
                    found_decimal_pt = 0;
                    dec_multiplier = 10;
                    frac_multiplier = 0.1f;
                    
                    continue;
                }
                
                if (file_buffer[file_buffer_index] == '-') {
                    is_negative = 1;
                }
                
                if (file_buffer[file_buffer_index] == '.') {
                    found_decimal_pt = 1;
                }
                
                int is_num =
                (file_buffer[file_buffer_index] > 47 &&
                 file_buffer[file_buffer_index] < 58);
                
                if (found_decimal_pt == 0 && is_num) {
                    num = num * dec_multiplier;
                    int temp = file_buffer[file_buffer_index] - 48;
                    num += temp;
                }
                
                if (found_decimal_pt == 1 && is_num) {
                    int temp = file_buffer[file_buffer_index] - 48;
                    float x = temp * frac_multiplier;
                    num += x;
                    frac_multiplier *= 0.1f;
                }
                
                file_buffer_index++;
                
                if (file_buffer[file_buffer_index] == 13 ||
                    file_buffer[file_buffer_index] == 10)
                {
                    if (is_negative == 1) {
                        num = -1 * num;
                    }
                    
                    val_buffer[value_buffer->index] = *(int *)&num;
                    value_buffer->index++;
                    
					nums_read++;
                    
					if (nums_read < 3) {
						val_buffer[value_buffer->index] = 0;
						value_buffer->index++;
					}
                }
            }
        }
        break;
		case FACE:
        {
            int *val_buffer = (int *)value_buffer->buffer;
            
            file_buffer_index = file_buffer_index+2;
            while (file_buffer[file_buffer_index] == ' ') {
                file_buffer_index++;
            }
            
            int dec_multiplier = 10;
            int num = 0;
            
            while(file_buffer[file_buffer_index] != 13 &&
                  file_buffer[file_buffer_index] != 10)
            {
                if (file_buffer[file_buffer_index] == ' ') {
                    
                    val_buffer[value_buffer->index] = num;
                    num = 0;
                    
                    file_buffer_index++;
                    value_buffer->index++;
                    
                    dec_multiplier = 10;
                    
                    continue;
                }
                
                int is_num =
                (file_buffer[file_buffer_index] > 47 &&
                 file_buffer[file_buffer_index] < 58);
                
                if (is_num) {
                    num = num * dec_multiplier;
                    int temp = file_buffer[file_buffer_index] - 48;
                    num += temp;
                }
                
                if (file_buffer[file_buffer_index] == '/') {
                    val_buffer[value_buffer->index] = num;
                    value_buffer->index++;
                    
                    num = 0;
                    dec_multiplier = 10;
                }
                
                file_buffer_index++;
                
                if (file_buffer[file_buffer_index] == 13 ||
                    file_buffer[file_buffer_index] == 10)
                {
                    val_buffer[value_buffer->index] = num;
                    value_buffer->index++;
                }
            }
        }
        break;
	}
    
	return value_buffer->index;
}

void iterate_file_fill_buffer(char *file_buffer, size_t file_size,
                              memory *vertex_geom, memory *vertex_tex,
                              memory *vertex_norm, memory *faces)
{
    for (size_t i = 0; i < file_size; i++) {
        if (file_buffer[i] == 'v' && file_buffer[i+1] == ' ') {
            vertex_geom->index =
                read_values_into_buffer(file_buffer, i, vertex_geom,
                                        GEOMETRIC_VERTICES);
        }
        
        if (file_buffer[i] == 'v' && file_buffer[i+1] == 't') {
            vertex_tex->index =
                read_values_into_buffer(file_buffer, i, vertex_tex,
                                        TEXTURE_VERTICES);
        }
        
        if (file_buffer[i] == 'v' && file_buffer[i+1] == 'n') {
            vertex_norm->index =
                read_values_into_buffer(file_buffer, i, vertex_norm,
                                        VERTEX_NORMAL);
        }
        
        if (file_buffer[i] == 'f' && file_buffer[i+1] == ' ') {
            faces->index =
                read_values_into_buffer(file_buffer, i, faces, FACE);
        }
    }
}

void gen_header_file(const char *header_guard, const char *header_file_name,
                     memory *vertex_geom, memory *vertex_norm, memory *vertex_tex,
                     memory *faces)
{
    memory header_file;
    header_file.size = vertex_geom->size;
    header_file.size += vertex_norm->size;
    header_file.size += vertex_tex->size;
	header_file.size *= 11; // 9 precision + 1 decimal + 1 '.'
    header_file.size += (faces->size * 10); // 10 max length for a 32 bit number
    header_file.index = 0;
    header_file.buffer = VirtualAlloc(0, header_file.size,
                                      MEM_COMMIT|MEM_RESERVE,
                                      PAGE_READWRITE);
    
    header_file.index =
        sprintf_s((char *)header_file.buffer, header_file.size,
                  "#ifndef %s\n", header_guard);
    header_file.index +=
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "#define %s\n\n", header_guard);
    
    int *buf = (int *)vertex_geom->buffer;
    header_file.index += 
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "Vertex vertex_geometric[] = {\n", 0);
    for (size_t i = 0; i < vertex_geom->index; i+=3) {
        header_file.index += 
            sprintf_s((char *)header_file.buffer + header_file.index,
                      header_file.size - header_file.index,
                      "{.x = %f, .y = %f, .z = %f},\n",
                      *(float *)&buf[i], *(float *)&buf[i + 1], *(float *)&buf[i + 2]);
    }
    header_file.index += 
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "};\n\n", 0);
    buf = (int *)vertex_norm->buffer;
    header_file.index += 
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "VertexNormal vertex_norm[] = {\n", 0);
    for (size_t i = 0; i < vertex_norm->index; i+=3) {
        header_file.index += 
            sprintf_s((char *)header_file.buffer + header_file.index,
                      header_file.size - header_file.index,
                      "{.i = %f, .j = %f, .k = %f},\n",
                      *(float *)&buf[i], *(float *)&buf[i + 1], *(float *)&buf[i + 2]);
    }
    header_file.index +=
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "};\n\n", 0);
    
    buf = (int *)vertex_tex->buffer;
    header_file.index +=
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "VertexTexture vertex_tex[] = {\n", 0);
    for (size_t i = 0; i < vertex_tex->index; i+=3) {
        header_file.index += 
            sprintf_s((char *)header_file.buffer + header_file.index,
                      header_file.size - header_file.index,
                      "{.tx = %f, .ty = %f, .tz = %f},\n",
                      *(float *)&buf[i], *(float *)&buf[i + 1], *(float *)&buf[i + 2]);
    }
    header_file.index +=
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "};\n\n", 0);
    
    int *buf2 = (int *)faces->buffer;
    header_file.index +=
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "Faces faces[] = {\n", 0);
    for (size_t i = 0; i < faces->index; i+=3) {
        header_file.index += 
            sprintf_s((char *)header_file.buffer + header_file.index,
                      header_file.size - header_file.index,
                      "{.vI = %d, .vtI = %d, .vnI = %d},\n",
                      buf2[i], buf2[i + 1], buf2[i + 2]);
    }
    header_file.index +=
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "};\n\n", 0);
    header_file.index +=
        sprintf_s((char *)header_file.buffer + header_file.index,
                  header_file.size - header_file.index,
                  "#endif", 0);
    
    char header_file_name_str[64];
    sprintf_s(header_file_name_str, 64, "../%s", header_file_name);
    HANDLE obj_h_file_handle =
        CreateFileA(header_file_name_str, GENERIC_WRITE, 0, 0,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    
    WriteFile(obj_h_file_handle, header_file.buffer,
              header_file.index, 0, 0);
    
    CloseHandle(obj_h_file_handle);
    
    VirtualFree(header_file.buffer, 0, MEM_RELEASE);
    
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev,
                     PSTR cmdline, int cmdshow) 
{
    obj_vertex_geometric.index = 0;
    obj_vertex_geometric.size = 10240 * 4 * 3;
    obj_vertex_geometric.buffer = VirtualAlloc(0,
                                               obj_vertex_geometric.size, 
                                               MEM_COMMIT|MEM_RESERVE,
                                               PAGE_READWRITE);
    
    obj_vertex_norm.index = 0;
    obj_vertex_norm.size = 10240 * 4 * 3;
    obj_vertex_norm.buffer = VirtualAlloc(0, obj_vertex_norm.size,
                                          MEM_COMMIT|MEM_RESERVE,
                                          PAGE_READWRITE);
    
    obj_vertex_tex.index = 0;
    obj_vertex_tex.size = 10240 * 4 * 3;
    obj_vertex_tex.buffer = VirtualAlloc(0, obj_vertex_tex.size,
                                         MEM_COMMIT|MEM_RESERVE,
                                         PAGE_READWRITE);
    
    obj_face.index = 0;
    obj_face.size = 10240 * 4 * 3;
    obj_face.buffer = VirtualAlloc(0, obj_face.size, MEM_COMMIT|MEM_RESERVE,
                                   PAGE_READWRITE);
    
    file obj_file;
    HANDLE file_handle;
    
    file_handle = read_file_into_buffer("obj/simple_medieval_house.obj", &obj_file);
    iterate_file_fill_buffer(obj_file.buffer, obj_file.size, &obj_vertex_geometric,
                             &obj_vertex_tex, &obj_vertex_norm, &obj_face);
    gen_header_file("HOUSE_H_", "simple_house_obj.h", &obj_vertex_geometric,
                    &obj_vertex_norm, &obj_vertex_tex, &obj_face);
    obj_vertex_geometric.index = 0;
    obj_vertex_tex.index = 0;
    obj_vertex_norm.index = 0;
    obj_face.index = 0;
    
    VirtualFree(obj_file.buffer, 0, MEM_RELEASE);
    CloseHandle(file_handle);
    
    file_handle = read_file_into_buffer("obj/african_head.obj", &obj_file);
    iterate_file_fill_buffer(obj_file.buffer, obj_file.size,
							 &obj_vertex_geometric,
                             &obj_vertex_tex, &obj_vertex_norm, &obj_face);
    gen_header_file("AFRICAN_FACE_H_", "african_face_obj.h",
					&obj_vertex_geometric,
                    &obj_vertex_norm, &obj_vertex_tex, &obj_face);
    obj_vertex_geometric.index = 0;
    obj_vertex_tex.index = 0;
    obj_vertex_norm.index = 0;
    obj_face.index = 0;
    
    VirtualFree(obj_file.buffer, 0, MEM_RELEASE);
    CloseHandle(file_handle);
    
    file_handle = read_file_into_buffer("obj/low_poly_cube.obj", &obj_file);
    iterate_file_fill_buffer(obj_file.buffer, obj_file.size,
							 &obj_vertex_geometric,
                             &obj_vertex_tex, &obj_vertex_norm, &obj_face);
    gen_header_file("CUBE_OBJ_H_", "cube_obj.h",  &obj_vertex_geometric,
                    &obj_vertex_norm, &obj_vertex_tex, &obj_face);
    obj_vertex_geometric.index = 0;
    obj_vertex_tex.index = 0;
    obj_vertex_norm.index = 0;
    obj_face.index = 0;
    
    VirtualFree(obj_file.buffer, 0, MEM_RELEASE);
    CloseHandle(file_handle);

    file_handle = read_file_into_buffer("obj/utah.obj", &obj_file);
    iterate_file_fill_buffer(obj_file.buffer, obj_file.size,
							 &obj_vertex_geometric,
                             &obj_vertex_tex, &obj_vertex_norm, &obj_face);
    gen_header_file("UTAH_OBJ_H_", "utah_obj.h",  &obj_vertex_geometric,
                    &obj_vertex_norm, &obj_vertex_tex, &obj_face);
    obj_vertex_geometric.index = 0;
    obj_vertex_tex.index = 0;
    obj_vertex_norm.index = 0;
    obj_face.index = 0;
    
    VirtualFree(obj_file.buffer, 0, MEM_RELEASE);
    CloseHandle(file_handle);

    VirtualFree(obj_vertex_geometric.buffer, 0, MEM_RELEASE);
    VirtualFree(obj_vertex_norm.buffer, 0, MEM_RELEASE);
    VirtualFree(obj_vertex_tex.buffer, 0, MEM_RELEASE);
    VirtualFree(obj_face.buffer, 0, MEM_RELEASE);
}
