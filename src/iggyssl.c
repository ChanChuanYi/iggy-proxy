/* -- iggyssl.c --
Author: Ignacio Llamas Avalos Jr
Class: CMPE 156 / Network Programming
Credits: 
        OpenSSL library calls are made possible by example code used in the 
        IBM OpenSSL tutorial authored by Kenneth Ballard 
        (kballard@kennethballard.com), Software Engineer, MediNotes Corp.
*/

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extra.h"
#include "iggyssl.h"

void secure_and_send(int client_fd, char* read_pipe, char* write_pipe, int index, FILE* d_out){
	BIO* bio;
	SSL* ssl;
	SSL_CTX* ctx;
	
	/*init ssl library*/
	SSL_library_init();
	ERR_load_BIO_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	
	/* Set up the SSL context */

    ctx = SSL_CTX_new(SSLv23_client_method());

    /* Load the trust store */

    if(! SSL_CTX_load_verify_locations(ctx, "cacerts.pem", NULL))
    {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return;
    }
    /* Setup the connection */

    bio = BIO_new_ssl_connect(ctx);

    /* Set the SSL_MODE_AUTO_RETRY flag */

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    
    /* Create and setup the connection */
	char host[LINE];
	sprintf(host,"%s:%s",safe_host[index],safe_port[index]);
    BIO_set_conn_hostname(bio, safe_host[index]);
    BIO_set_conn_port(bio,safe_port[index]);

    if(BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Error attempting to connect\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return;
    }

    /* Check the certificate */

    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
        fprintf(stderr, "Certificate verification error: %lu\n", SSL_get_verify_result(ssl));
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return;
    }
    /* Send the request */

    BIO_write(bio, read_pipe, strlen(read_pipe));

    /* Read in the response */

    for(;;)
    {
        int p = BIO_read(bio, write_pipe, PIPE_MAX);
        if(p <= 0) break;
        p = write(client_fd,write_pipe,p);
    }

    /* Close the connection and free the context */
	
    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    close(client_fd);
    fclose(d_out);
    return;
}

