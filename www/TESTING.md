# Webserv browser test surface

The source of truth is `conf/complete_webserv.conf`. Start Webserv from the repository root so its relative roots resolve correctly.

| Feature | Endpoint | Method | Expected |
|---|---|---|---|
| Main home | `/` on `:8081` | GET | 200 |
| Static file | `/hello.txt` | GET | 200 |
| Missing resource | `/missing-page` | GET | 404 |
| Public location | `/public/` | GET | 200 |
| Raw upload | `/upload/name.txt` | POST | 201 |
| Multipart upload | `/upload` | POST | 201 |
| Upload listing | `/uploads/` | GET | 200 |
| Redirect | `/old` | GET | 301 to `/` |
| Method restriction | `/` | POST | 405 |
| Oversize body | `/upload/oversize-probe.txt` | POST | 413 above 1,000,000 bytes |
| Python CGI | `/cgi-bin/test.py` | GET | 200 when CGI succeeds |
| Slow Python CGI | `/cgi-bin/dos.py` | GET | CGI timeout path |
| Secondary home | `/` on `:8082` | GET | 200 |
| Secondary static file | `/status.txt` on `:8082` | GET | 200 |

## Known implementation limits

- DELETE is accepted by configuration parsing but its C++ handler is currently commented out.
- Query-string separation is still pending, so the CGI dashboard deliberately uses a path without a query.
- The configuration declares custom pages for 400, 403, 404, 405, 413, 500, 501, and 505; all are present under `www/errors/`.
