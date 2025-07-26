/* gimp-1fpirate.c - GIMP plugin to load "1fpirate" format (1-bit farbfeld variant)
 *
 * This plugin loads monochrome .1f or .1ff images with the "1fpirate" magic header.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <libgimp/gimp.h>
#include <arpa/inet.h>

#define MAGIC "1fpirate"
#define LOAD_PROC "file_1fpirate_load"
#define SAVE_PROC "file_1fpirate_save"

#define extensions "1f,1ff,1fpirate"

static void query(void);
static void run(const gchar *name, gint nparams, const GimpParam *param,
                gint *nreturn_vals, GimpParam **return_vals);

GimpPlugInInfo PLUG_IN_INFO = {
  NULL,
  NULL,
  query,
  run
};

MAIN()

static void query(void) {
  static const GimpParamDef load_args[] = {
    { GIMP_PDB_INT32, "run-mode", "Run mode" },
    { GIMP_PDB_STRING, "filename", "The name of the file to load" },
    { GIMP_PDB_STRING, "raw-filename", "The name entered" },
  };

  static const GimpParamDef load_return_vals[] = {
    { GIMP_PDB_IMAGE, "image", "Output image" }
  };

  gimp_install_procedure(
    "file-1fpirate-load",
    "Loads images in 1fpirate (1-bit farbfeld variant) format.",
    "Loads a monochrome 1fpirate format image.",
    "You",
    "GPL",
    "2025",
    "1fpirate image",
    NULL,
    GIMP_PLUGIN,
    G_N_ELEMENTS(load_args), G_N_ELEMENTS(load_return_vals),
    load_args, load_return_vals
  );

  gimp_register_file_handler_mime("file-1fpirate-load", "image/x-1fpirate");
  /*gimp_register_magic_load_handler(
           LOAD_PROC,
	   extensions,
	   "",
	   0,
	   MAGIC);*/
  gimp_register_magic_load_handler(
	    "file-1fpirate-load",
	    "1f,1ff,1fpirate", /*"1f,1ff*/
	    "",
	    //0,
	    MAGIC);
}

static gint32 load_image(const gchar *filename) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) return -1;

  char magic[8];
  fread(magic, 1, 8, fp);
  if (memcmp(magic, MAGIC, 8) != 0) {
    fclose(fp);
    return -1;
  }

  uint32_t width, height;
  fread(&width, 4, 1, fp);
  fread(&height, 4, 1, fp);
  width = ntohl(width);
  height = ntohl(height);

  gint32 image = gimp_image_new(width, height, GIMP_RGB);
  gint32 layer = gimp_layer_new(image, "1fpirate layer", width, height,
                                /*GIMP_RGB_IMAGE*/ GIMP_GRAY_IMAGE, 100, GIMP_NORMAL_MODE);
  gimp_image_insert_layer(image, layer, -1, 0);

  GimpDrawable *drawable = gimp_drawable_get(layer);
  GimpPixelRgn region;
  gimp_pixel_rgn_init(&region, drawable, 0, 0, width, height, TRUE, FALSE);

  guchar *row = g_new(guchar, width * 3);
  int row_bytes = (width + 7) / 8;

  for (uint32_t y = 0; y < height; y++) {
    fread(row, 1, row_bytes, fp);
    guchar *line = g_new(guchar, width * 3);
    for (uint32_t x = 0; x < width; x++) {
      int byte_index = x / 8;
      int bit_index = 7 - (x % 8);
      int bit = (row[byte_index] >> bit_index) & 1;
      //guchar val = bit ? 0 : 255;
      guchar val = bit ? 255 : 0; /* // now: 1 = white, 0 = black*/
      line[x * 3 + 0] = val;
      line[x * 3 + 1] = val;
      line[x * 3 + 2] = val;
    }
    gimp_pixel_rgn_set_row(&region, line, 0, y, width);
    g_free(line);
  }
  gimp_invert(layer);

  g_free(row);
  gimp_drawable_flush(drawable);
  gimp_drawable_detach(drawable);
  fclose(fp);
  return image;
}

static void run(const gchar *name, gint nparams, const GimpParam *param,
                gint *nreturn_vals, GimpParam **return_vals) {
  static GimpParam values[2];
  *nreturn_vals = 2;
  *return_vals = values;

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_SUCCESS;

  gint32 image = load_image(param[1].data.d_string);
  if (image != -1) {
    values[1].type = GIMP_PDB_IMAGE;
    values[1].data.d_image = image;
  } else {
    values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
  }
}
