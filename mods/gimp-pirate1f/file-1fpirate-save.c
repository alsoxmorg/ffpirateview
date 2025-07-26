#include <libgimp/gimp.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#define MAGIC "1fpirate"
#define EXTENSIONS "1f,1ff"
#define SAVE_PROC "file-1fpirate-save"

/* Write 1-bit packed data */
static void write_1bit_data(FILE *fp, GimpDrawable *drawable, gint width, gint height) {
    GimpPixelRgn rgn;
    guchar *row = g_new(guchar, width);
    gimp_pixel_rgn_init(&rgn, drawable, 0, 0, width, height, FALSE, FALSE);

    for (gint y = 0; y < height; y++) {
        gimp_pixel_rgn_get_row(&rgn, row, 0, y, width);

        uint8_t byte = 0;
        int bit_pos = 0;

        for (gint x = 0; x < width; x++) {
            guchar v = row[x]; // assuming grayscale or 1-bit image
            int bit = v < 128 ? 1 : 0; // black = 1, white = 0
            byte |= (bit << (7 - bit_pos));
            bit_pos++;

            if (bit_pos == 8) {
                fputc(byte, fp);
                byte = 0;
                bit_pos = 0;
            }
        }

        // Flush remaining bits at end of row
        if (bit_pos != 0) {
            fputc(byte, fp);
        }
    }

    g_free(row);
}

/* Save function */
static gint32 save_1fpirate(const gchar *filename, const gchar *raw_filename,
                            gint32 image_ID, gint32 drawable_ID, GError **error) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errno),
                    "Unable to open output file");
        return -1;
    }

    GimpDrawable *drawable = gimp_drawable_get(drawable_ID);
    gint width = gimp_image_width(image_ID);
    gint height = gimp_image_height(image_ID);

    fwrite(MAGIC, 1, 8, fp);

    uint32_t w_net = htonl(width);
    uint32_t h_net = htonl(height);
    fwrite(&w_net, 4, 1, fp);
    fwrite(&h_net, 4, 1, fp);

    write_1bit_data(fp, drawable, width, height);

    fclose(fp);
    gimp_drawable_detach(drawable);
    return image_ID;
}

/* Register the plugin */
void query(void) {
    static GimpParamDef args[] = {
        { GIMP_PDB_INT32,    "run-mode",      "Run mode" },
        { GIMP_PDB_IMAGE,    "image",         "Input image" },
        { GIMP_PDB_DRAWABLE, "drawable",      "Drawable to save" },
        { GIMP_PDB_STRING,   "filename",      "Output file name" },
        { GIMP_PDB_STRING,   "raw-filename",  "Raw file name" }
    };

    gimp_install_procedure(
        SAVE_PROC,
        "Save 1fpirate image",
        "Export an image to the 1fpirate 1-bit packed format",
        "tcooper",
        "public domain",
        "2025",
        NULL, // No menu entry
        "RGB*, GRAY*",
        GIMP_PLUGIN,
        G_N_ELEMENTS(args), 0,
        args, NULL
    );

    /*gimp_register_save_handler(SAVE_PROC, "1f,1ff", "");*/
    gimp_register_file_handler_mime(SAVE_PROC, "application/x-1fpirate");
    gimp_register_save_handler(SAVE_PROC, "1f,1ff", "1fpirate");
    //gimp_register_file_handler_name(SAVE_PROC, "1fpirate image");
}

void run(const gchar *name, gint nparams, const GimpParam *param,
         gint *nreturn_vals, GimpParam **return_vals) {

    static GimpParam values[1];
    *nreturn_vals = 1;
    *return_vals = values;
    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_SUCCESS;

    if (strcmp(name, SAVE_PROC) == 0) {
        GError *error = NULL;
        if (save_1fpirate(param[3].data.d_string,
                          param[4].data.d_string,
                          param[1].data.d_image,
                          param[2].data.d_drawable,
                          &error) == -1) {
            values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
        }
    }
}
