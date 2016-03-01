#include "aos_util.h"
#include "aos_log.h"

static const char *g_s_wday[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char *g_s_mon[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char g_s_gmt_format[] = "%s, %.2d %s %.4d %.2d:%.2d:%.2d GMT";

int aos_parse_xml_body(aos_list_t *bc, xmlDoc **doc_, xmlNode **root)
{
    int res;
    aos_buf_t *b;    
    xmlDoc *doc;
    xmlParserCtxt *ctxt;
    *root = NULL;

    ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);
    if (ctxt == NULL) {
        aos_error_log("Failed to create parser context.");
        return AOSE_INTERNAL_ERROR;
    }

    aos_list_for_each_entry(b, bc, node) {
        xmlParseChunk(ctxt, (char *)b->pos, aos_buf_size(b), 0);
    }
    xmlParseChunk(ctxt, NULL, 0, 1);
    
    doc = ctxt->myDoc;
    res = ctxt->wellFormed;
    xmlFreeParserCtxt(ctxt);
    if (!res) {
        goto error_exit;
    }
    
    *root = xmlDocGetRootElement(doc);
    if (*root == NULL) {
        goto error_exit;
    }
    
    *doc_ = doc;
    return AOSE_OK;

error_exit:
    xmlFreeDoc(doc);
    aos_error_log("Failed to parse xml.");
    return AOSE_INTERNAL_ERROR;
}

int aos_convert_to_gmt_time(char* date, const char* format, apr_time_exp_t *tm)
{
    int size = apr_snprintf(date, AOS_MAX_GMT_TIME_LEN, format, 
        g_s_wday[tm->tm_wday], tm->tm_mday, g_s_mon[tm->tm_mon], 1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
    if (size >= 0 && size < AOS_MAX_GMT_TIME_LEN) {
        return AOSE_OK;
    } else {
        return AOSE_INTERNAL_ERROR;
    }
}

int aos_get_gmt_str_time(char datestr[AOS_MAX_GMT_TIME_LEN])
{
    int s;
    apr_time_t now;
    char buf[128];
    apr_size_t retsize = 0;
    apr_time_exp_t result;

    now = apr_time_now();
    if ((s = apr_time_exp_gmt(&result, now)) != APR_SUCCESS) {
        aos_error_log("apr_time_exp_gmt fialure, code:%d %s.", s, apr_strerror(s, buf, sizeof(buf)));
        return AOSE_INTERNAL_ERROR;
    }
    
    if ((s = aos_convert_to_gmt_time(datestr, g_s_gmt_format, &result))
        != AOSE_OK) {
        aos_error_log("aos_convert_to_GMT failure, code:%d.", s);
    }

    return s;
}

int aos_url_encode(char *dest, const char *src, int maxSrcSize)
{
    static const char *hex = "0123456789ABCDEF";

    int len = 0;
    unsigned char c;

    while (*src) {
        if (++len > maxSrcSize) {
            *dest = 0;
            return AOSE_INVALID_ARGUMENT;
        }
        c = *src;
        if (isalnum(c) ||
            (c == '-') || (c == '_') || (c == '.') || (c == '!') || 
            (c == '~') || (c == '*') || (c == '\'') || (c == '(') ||
            (c == ')') || (c == '/')) {
            *dest++ = c;
        } else if (*src == ' ') {
            // *dest++ = '+';
            *dest++ = '%';
            *dest++ = '2';
            *dest++ = '0';
        } else {
            *dest++ = '%';
            *dest++ = hex[c >> 4];
            *dest++ = hex[c & 15];
        }
        src++;
    }

    *dest = 0;

    return AOSE_OK;
}

int aos_query_params_to_string(aos_pool_t *p, aos_table_t *query_params, aos_string_t *querystr)
{
    int rs;
    int pos;
    int len;
    char sep = '?';
    char ebuf[AOS_MAX_QUERY_ARG_LEN*3+1];
    char abuf[AOS_MAX_QUERY_ARG_LEN*6+128];
    int max_len;
    const aos_array_header_t *tarr;
    const aos_table_entry_t *telts;
    aos_buf_t *querybuf;

    if (apr_is_empty_table(query_params)) {
        return AOSE_OK;
    }

    max_len = sizeof(abuf)-1;
    querybuf = aos_create_buf(p, 256);
    aos_str_null(querystr);

    tarr = aos_table_elts(query_params);
    telts = (aos_table_entry_t*)tarr->elts;
    
    for (pos = 0; pos < tarr->nelts; ++pos) {
        if ((rs = aos_url_encode(ebuf, telts[pos].key, AOS_MAX_QUERY_ARG_LEN)) != AOSE_OK) {
            aos_error_log("query params args too big, key:%s.", telts[pos].key);
            return AOSE_INVALID_ARGUMENT;
        }
        len = apr_snprintf(abuf, max_len, "%c%s", sep, ebuf);
        if (telts[pos].val != NULL && *telts[pos].val != '\0') {
            if ((rs = aos_url_encode(ebuf, telts[pos].val, AOS_MAX_QUERY_ARG_LEN)) != AOSE_OK) {
                aos_error_log("query params args too big, value:%s.", telts[pos].val);
                return AOSE_INVALID_ARGUMENT;
            }
            len += apr_snprintf(abuf+len, max_len-len, "=%s", ebuf);
            if (len >= AOS_MAX_QUERY_ARG_LEN) {
                aos_error_log("query params args too big, %s.", abuf);
                return AOSE_INVALID_ARGUMENT;
            }
        }
        aos_buf_append_string(p, querybuf, abuf, len);
        sep = '&';
    }

    // result
    querystr->data = (char *)querybuf->pos;
    querystr->len = aos_buf_size(querybuf);
    
    return AOSE_OK;
}

void aos_gnome_sort(const char **headers, int size)
{
    const char *tmp;
    int i = 0, last_highest = 0;

    while (i < size) {
        if ((i == 0) || apr_strnatcasecmp(headers[i-1], headers[i]) < 0) {
            i = ++last_highest;
        } else {
            tmp = headers[i];
            headers[i] = headers[i - 1];
            headers[--i] = tmp;
        }
    }
}

const char* aos_http_method_to_string(http_method_e method)
{
    switch (method) {
        case HTTP_GET:
            return "GET";
        case HTTP_HEAD:
            return "HEAD";
        case HTTP_PUT:
            return "PUT";
        case HTTP_POST:
            return "POST";
        case HTTP_DELETE:
            return "DELETE";
        default:
            return "UNKNOWN";
    }
}

int aos_base64_encode(const unsigned char *in, int inLen, char *out)
{
    static const char *ENC = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    char *original_out = out;

    while (inLen) {
        // first 6 bits of char 1
        *out++ = ENC[*in >> 2];
        if (!--inLen) {
            // last 2 bits of char 1, 4 bits of 0
            *out++ = ENC[(*in & 0x3) << 4];
            *out++ = '=';
            *out++ = '=';
            break;
        }
        // last 2 bits of char 1, first 4 bits of char 2
        *out++ = ENC[((*in & 0x3) << 4) | (*(in + 1) >> 4)];
        in++;
        if (!--inLen) {
            // last 4 bits of char 2, 2 bits of 0
            *out++ = ENC[(*in & 0xF) << 2];
            *out++ = '=';
            break;
        }
        // last 4 bits of char 2, first 2 bits of char 3
        *out++ = ENC[((*in & 0xF) << 2) | (*(in + 1) >> 6)];
        in++;
        // last 6 bits of char 3
        *out++ = ENC[*in & 0x3F];
        in++, inLen--;
    }

    return (out - original_out);
}

// HMAC-SHA-1:
//
// K - is key padded with zeros to 512 bits
// m - is message
// OPAD - 0x5c5c5c...
// IPAD - 0x363636...
//
// HMAC(K,m) = SHA1((K ^ OPAD) . SHA1((K ^ IPAD) . m))
void HMAC_SHA1(unsigned char hmac[20], const unsigned char *key, int key_len,
               const unsigned char *message, int message_len)
{
    unsigned char kopad[64], kipad[64];
    int i;
    unsigned char digest[APR_SHA1_DIGESTSIZE];
    apr_sha1_ctx_t context;
    
    if (key_len > 64) {
        key_len = 64;
    }

    for (i = 0; i < key_len; i++) {
        kopad[i] = key[i] ^ 0x5c;
        kipad[i] = key[i] ^ 0x36;
    }

    for ( ; i < 64; i++) {
        kopad[i] = 0 ^ 0x5c;
        kipad[i] = 0 ^ 0x36;
    }

    apr_sha1_init(&context);
    apr_sha1_update(&context, (const char *)kipad, 64);
    apr_sha1_update(&context, (const char *)message, (unsigned int)message_len);
    apr_sha1_final(digest, &context);

    apr_sha1_init(&context);
    apr_sha1_update(&context, (const char *)kopad, 64);
    apr_sha1_update(&context, (const char *)digest, 20);
    apr_sha1_final(hmac, &context);
}