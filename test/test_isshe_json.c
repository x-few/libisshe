
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "isshe_file.h"
#include "isshe_json.h"
#include "isshe_unistd.h"


/* Used by some code below as an example datatype. */
struct record
{
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
};


/* Create a bunch of objects as demonstration. */
static int print_preallocated(isshe_json_t *root)
{
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    out = isshe_json_print(root);

    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(out) + 5;
    buf = (char*)malloc(len);
    if (buf == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* create buffer to fail */
    len_fail = strlen(out);
    buf_fail = (char*)malloc(len_fail);
    if (buf_fail == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* Print to buffer */
    if (!isshe_json_print_pre_allocated(root, buf, (int)len, 1)) {
        printf("isshe_json_print_pre_allocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("isshe_json_print_pre_allocated not the same as isshe_json_print!\n");
            printf("isshe_json_print result:\n%s\n", out);
            printf("isshe_json_print_pre_allocated result:\n%s\n", buf);
        }
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (isshe_json_print_pre_allocated(root, buf_fail, (int)len_fail, 1)) {
        printf("isshe_json_print_pre_allocated failed to show error with insufficient memory!\n");
        printf("isshe_json_print result:\n%s\n", out);
        printf("isshe_json_print_pre_allocated result:\n%s\n", buf_fail);
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    free(out);
    free(buf_fail);
    free(buf);
    return 0;
}

/* Create a bunch of objects as demonstration. */
static void test_create_objects(void)
{
    /* declare a few. */
    isshe_json_t *root = NULL;
    isshe_json_t *fmt = NULL;
    isshe_json_t *img = NULL;
    isshe_json_t *thm = NULL;
    isshe_json_t *fld = NULL;
    int i = 0;

    /* Our "days of the week" array: */
    const char *strings[7] =
    {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };
    /* Our matrix: */
    int numbers[3][3] =
    {
        {0, -1, 0},
        {1, 0, 0},
        {0 ,0, 1}
    };
    /* Our "gallery" item: */
    int ids[4] = { 116, 943, 234, 38793 };
    /* Our array of "records": */
    struct record fields[2] =
    {
        {
            "zip",
            37.7668,
            -1.223959e+2,
            "",
            "SAN FRANCISCO",
            "CA",
            "94107",
            "US"
        },
        {
            "zip",
            37.371991,
            -1.22026e+2,
            "",
            "SUNNYVALE",
            "CA",
            "94085",
            "US"
        }
    };
    volatile double zero = 0.0;

    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
    root = isshe_json_create_object();
    isshe_json_add_item_to_object(root, "name", isshe_json_create_string("Jack (\"Bee\") Nimble"));
    isshe_json_add_item_to_object(root, "format", fmt = isshe_json_create_object());
    isshe_json_add_string_to_object(fmt, "type", "rect");
    isshe_json_add_number_to_object(fmt, "width", 1920);
    isshe_json_add_number_to_object(fmt, "height", 1080);
    isshe_json_add_false_to_object (fmt, "interlace");
    isshe_json_add_number_to_object(fmt, "frame rate", 24);

    /* Print to text */
    if (print_preallocated(root) != 0) {
        isshe_json_delete(root);
        exit(EXIT_FAILURE);
    }
    isshe_json_delete(root);

    /* Our "days of the week" array: */
    root = isshe_json_create_string_array(strings, 7);

    if (print_preallocated(root) != 0) {
        isshe_json_delete(root);
        exit(EXIT_FAILURE);
    }
    isshe_json_delete(root);

    /* Our matrix: */
    root = isshe_json_create_array();
    for (i = 0; i < 3; i++)
    {
        isshe_json_add_item_to_array(root, isshe_json_create_int_array(numbers[i], 3));
    }

    /* cJSON_ReplaceItemInArray(root, 1, isshe_json_create_string("Replacement")); */

    if (print_preallocated(root) != 0) {
        isshe_json_delete(root);
        exit(EXIT_FAILURE);
    }
    isshe_json_delete(root);

    /* Our "gallery" item: */
    root = isshe_json_create_object();
    isshe_json_add_item_to_object(root, "Image", img = isshe_json_create_object());
    isshe_json_add_number_to_object(img, "Width", 800);
    isshe_json_add_number_to_object(img, "Height", 600);
    isshe_json_add_string_to_object(img, "Title", "View from 15th Floor");
    isshe_json_add_item_to_object(img, "Thumbnail", thm = isshe_json_create_object());
    isshe_json_add_string_to_object(thm, "Url", "http:/*www.example.com/image/481989943");
    isshe_json_add_number_to_object(thm, "Height", 125);
    isshe_json_add_string_to_object(thm, "Width", "100");
    isshe_json_add_item_to_object(img, "IDs", isshe_json_create_int_array(ids, 4));

    if (print_preallocated(root) != 0) {
        isshe_json_delete(root);
        exit(EXIT_FAILURE);
    }
    isshe_json_delete(root);

    /* Our array of "records": */
    root = isshe_json_create_array();
    for (i = 0; i < 2; i++)
    {
        isshe_json_add_item_to_array(root, fld = isshe_json_create_object());
        isshe_json_add_string_to_object(fld, "precision", fields[i].precision);
        isshe_json_add_number_to_object(fld, "Latitude", fields[i].lat);
        isshe_json_add_number_to_object(fld, "Longitude", fields[i].lon);
        isshe_json_add_string_to_object(fld, "Address", fields[i].address);
        isshe_json_add_string_to_object(fld, "City", fields[i].city);
        isshe_json_add_string_to_object(fld, "State", fields[i].state);
        isshe_json_add_string_to_object(fld, "Zip", fields[i].zip);
        isshe_json_add_string_to_object(fld, "Country", fields[i].country);
    }

    /* cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root, 1), "City", isshe_json_create_int_array(ids, 4)); */

    if (print_preallocated(root) != 0) {
        isshe_json_delete(root);
        exit(EXIT_FAILURE);
    }
    isshe_json_delete(root);

    root = isshe_json_create_object();
    isshe_json_add_number_to_object(root, "number", 1.0 / zero);

    if (print_preallocated(root) != 0) {
        isshe_json_delete(root);
        exit(EXIT_FAILURE);
    }
    isshe_json_delete(root);
}


void test_read_json_file(char *file)
{
    isshe_fd_t      fd;
    ssize_t         len;
    char            *buf;

    // 打开文件
    fd = isshe_open(file, ISSHE_FILE_RDONLY);

    // 读取文件
    buf = isshe_read_all(fd, &len);
    if (!buf) {
        printf("icnfig_parse error: isshe_read_all\n");
        exit(0);
    }
    // 解析json
    isshe_json_t* json = isshe_json_parse(buf);
    if (!json) {
        printf("icnfig_parse error: json parse failed\n");
        exit(0);
    }

    isshe_free(buf);
    buf = isshe_json_print(json);
    printf("%s\n", buf);

    isshe_json_delete(json);
    isshe_free(buf);
    isshe_close(fd);
}

int main()
{
    /* print the version */
    printf("Version: %s\n", isshe_json_version());

    char *file = "./config.json";
    test_read_json_file(file);

    test_create_objects();
}

