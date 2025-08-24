#ifndef AIO_REQUEST_H
#define AIO_REQUEST_H

#include <aio.h>
#include <stdlib.h>

typedef struct __aio_request
{
    int conn;
    char *type;
    struct aiocb *cb_ptr;
} aio_request;

#endif // AIO_REQUEST_H

#define BUFFER_SIZE 255

aio_request *create_aio_request(int conn, char *type, int fd)
{
    aio_request *req = (aio_request *)malloc(sizeof(aio_request));
    struct aiocb *cb_ptr = (struct aiocb *)malloc(sizeof(struct aiocb));

    char *buffer = (char *)malloc(BUFFER_SIZE);

    cb_ptr->aio_fildes = fd;
    cb_ptr->aio_offset = 0;
    cb_ptr->aio_nbytes = BUFFER_SIZE;
    cb_ptr->aio_buf = buffer;

    req->conn = conn;
    req->type = type;
    req->cb_ptr = cb_ptr;

    return req;
}

void free_aio_request(aio_request *req)
{
    free((void *)req->cb_ptr->aio_buf);
    free(req->cb_ptr);
    free(req);
}