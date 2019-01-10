# Configuration files

## SSL

To enable https connection to your server you will need a valid ssl
certificate for your device. This certificate can be included as part of
the programmes `.text`. It is included with an asm call to the start of the
memory block where the cerificate is stored via an  binary data symbols.
The content is pointed to by an `external char[]` variable defined in
`config_ssl.h`. The file name to be stored as the binary data symbol is
specified in `component.mk`. By default the certificate file is called
`ssl_com_root_cert.pem` in the main directory.


