
#include "isshe_common.h"

int isshe_rand_bytes_dev_urandom(isshe_char_t *buf, isshe_int_t len)
{
    int res;
    FILE *fp;

    fp = fopen(ISSHE_DEV_URANDOM, "rb");
    if (!fp) {
        return ISSHE_FAILURE;
    }

    res = fread(buf, 1, len, fp);
    fclose(fp);

    if (res != len) {
        return ISSHE_FAILURE;
    }

    return ISSHE_SUCCESS;
}

int isshe_rand_bytes(isshe_char_t *buf, isshe_int_t len)
{
    return isshe_rand_bytes_dev_urandom(buf, len);
}