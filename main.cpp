/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Ian Craggs - make sure QoS2 processing works, and add device headers
 *******************************************************************************/

 /**
  This is a sample program to illustrate the use of the MQTT Client library
  on the mbed platform.  The Client class requires two classes which mediate
  access to system interfaces for networking and timing.  As long as these two
  classes provide the required public programming interfaces, it does not matter
  what facilities they use underneath. In this program, they use the mbed
  system libraries.

 */

#include "mbed.h"
#include "rtos.h"
#undef MBED_CONF_APP_ESP8266_DEBUG
#include "easy-connect.h"
#include "MQTTThreadedClient.h"
#include "mbedtls/platform.h"

using namespace MQTT;

Serial pc(USBTX, USBRX);
// Thread msgSender(osPriorityNormal, 1 * 2);


/* List of trusted root CA certificates
 * currently only "letsencrypt", the CA for mbedhacks.com
 *
 * To add more than one root, just concatenate them.
 *
 * TODO: Move this certificate file onto the SD card.
 */
static const char *TLS_CA_PEM =   
   "-----BEGIN CERTIFICATE-----\n"
   "MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\n"
   "yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\n"
   "ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\n"
   "U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\n"
   "ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\n"
   "aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\n"
   "MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\n"
   "ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\n"
   "biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\n"
   "U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\n"
   "aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\n"
   "nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\n"
   "t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\n"
   "SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\n"
   "BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\n"
   "rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\n"
   "NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\n"
   "BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\n"
   "BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\n"
   "aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\n"
   "MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\n"
   "p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\n"
   "5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\n"
   "WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\n"
   "4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\n"
   "hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\n"
   "-----END CERTIFICATE-----\n";
   // "-----BEGIN CERTIFICATE-----\n"
   // "MIIEDzCCAvegAwIBAgIJAPFeIdqhSuyWMA0GCSqGSIb3DQEBCwUAMIGdMQswCQYD\n"
   // "VQQGEwJVUzETMBEGA1UECAwKQ2FsaWZvcm5pYTERMA8GA1UEBwwIU2FuIEpvc2Ux\n"
   // "DDAKBgNVBAoMA0FSTTEaMBgGA1UECwwRSVNHLURhdGEtU2VydmljZXMxFjAUBgNV\n"
   // "BAMMDWRhdGEubWJlZC5jb20xJDAiBgkqhkiG9w0BCQEWFWFyZGFtYW4uc2luZ2hA\n"
   // "YXJtLmNvbTAeFw0xODA2MDYxODI2NTVaFw0yODA2MDMxODI2NTVaMIGdMQswCQYD\n"
   // "VQQGEwJVUzETMBEGA1UECAwKQ2FsaWZvcm5pYTERMA8GA1UEBwwIU2FuIEpvc2Ux\n"
   // "DDAKBgNVBAoMA0FSTTEaMBgGA1UECwwRSVNHLURhdGEtU2VydmljZXMxFjAUBgNV\n"
   // "BAMMDWRhdGEubWJlZC5jb20xJDAiBgkqhkiG9w0BCQEWFWFyZGFtYW4uc2luZ2hA\n"
   // "YXJtLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMmP837LvAjP\n"
   // "metEx5lutAenEA229x64XwGh3chcudWuMQ9X/Vl0CZT9ApoWXudSQHPicoF88aUo\n"
   // "1gfql2eMx4MpRi8bG3SvIghTWv7Xw0OrXx3UkTHEsKx/yleLm+LLdHPL9NcT9MXo\n"
   // "s9J85O9p0mtP+Ay7gT+ORZ8wd2o3RteWIVvOkpjNt0imj50OpfBe5dQswW3qPZZB\n"
   // "yk5daR2R5Tfwsd+2Q/qBYVrrMIbaKfxXdFqnRwvlRWEIHyCqYgrHTHs+CTvO31WN\n"
   // "CqMbfbUu+jJHXRP8/PIUNF0kc7mUkzpnD+GiStiI5PjI99IQUfn5M9jhJOmR8Uku\n"
   // "praSg8ypj6cCAwEAAaNQME4wHQYDVR0OBBYEFDPuZGkCND88XtnCy6Buo2DGSyZu\n"
   // "MB8GA1UdIwQYMBaAFDPuZGkCND88XtnCy6Buo2DGSyZuMAwGA1UdEwQFMAMBAf8w\n"
   // "DQYJKoZIhvcNAQELBQADggEBAKQbJGmCG6MuvPFwF59jFhLIxeDX/me+uNN1k51p\n"
   // "Q78Ifj7PCiVa5yfJk5GeKdeUROUS3F3SJtNUsVTeYWrj5CC1i+pWvcKm+XqjijNJ\n"
   // "m3b48D+/ML0SIc4QlVkfrJiB9AOkgfjZjCtX3b9hq7pa704CZMbh0r2MPqzKMfPj\n"
   // "P0hhkzeBKNdL0mEE8sepYD4xnRXWEbxv+s+AmRzqBc3lv/fjubtfePD838z/0wMt\n"
   // "ayBUreJMq1tj1cxB28Uc8P/QN11GFmlfB5yEJ5yuFiYMaTnJfZRb95JgbLU2BcPR\n"
   // "HcdNNA4A/WCNQJQoapecQ4RTkYQBA+5vKrmGoaBRTjRKDEY=\n"
   // "-----END CERTIFICATE-----\n";

const char *TLS_CLIENT_CERT = 
   "-----BEGIN CERTIFICATE-----\n"
   "MIIDWjCCAkKgAwIBAgIVALIkMUYWQxNQaXGOQh24Ek3d/3gZMA0GCSqGSIb3DQEB\n"
   "CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"
   "IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xODA2MDcyMTM2\n"
   "MTZaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"
   "dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDN5CYF5fzNuJ/gVw05\n"
   "o7c6t3pSRd1RdydASIcDAX12LiB9gN/9U1T1zvVrXrX/pnExxkStuWkSMX6IAJl0\n"
   "EpJQkeFf9DxIen5IXCdtOzWDo42qRa2Rd58CcCGVujeZkpjT1JuB4M4fTbDKPWVg\n"
   "yr+588lz2y49pI7Iqi7C9kn+gjFAlnsyjJbPXBFyFfe+USt7NsTRfoYHHYOFTRSK\n"
   "QcIPmgiWUmOpsKYIRCOOR7CEDqJhvu6z1KkZnWMK6T9G2zcANdPkrH7hDMdg/TN+\n"
   "asD9ZFR19wzFk5x4xlbcxRXlHVFpX3m+nCdw2tY9/qgPee3O7f0PXFz2XGYQT2eN\n"
   "ZYP/AgMBAAGjYDBeMB8GA1UdIwQYMBaAFDLSnffyj59k3nGan+2tiBV9egsYMB0G\n"
   "A1UdDgQWBBSwQDXUL245INwREJ4BJ0bZeqSoATAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"
   "DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAzqQQSyTvXxnQDij3xEFt72vG\n"
   "6mnl+XoyEPACTpQFgdQBpQYHUswZpKxLn28POYhT0CHqkMvfeAIAuFkgsw01Y8YZ\n"
   "cBMArKHO/neKin9Hd+aMRcEAuOxctgd3KI+NtkIWt0C3MFJYjfqP6R54QfpNRqLQ\n"
   "iIjjLXFHECOBoOt4SiJadZeJGsfLzEh0JdC/8gKus1mKA/aoDgW0vnm2utQMI9/a\n"
   "UHckDuDgai+tjTMqDO3m/ug0WcFNUyT869Vt5YLsqtXgCC92xP8HwCmNyy2qv+jk\n"
   "yHAG+c4cuoikNQrvo4E1G0bBTscc2UtnHreEy9KuO+PiNF7+8qShKY12thXAlw==\n"
   "-----END CERTIFICATE-----\n";
   // "-----BEGIN CERTIFICATE-----\n"
   // "MIIDwzCCAqsCCQDli0EDhASvHTANBgkqhkiG9w0BAQsFADCBnTELMAkGA1UEBhMC\n"
   // "VVMxEzARBgNVBAgMCkNhbGlmb3JuaWExETAPBgNVBAcMCFNhbiBKb3NlMQwwCgYD\n"
   // "VQQKDANBUk0xGjAYBgNVBAsMEUlTRy1EYXRhLVNlcnZpY2VzMRYwFAYDVQQDDA1k\n"
   // "YXRhLm1iZWQuY29tMSQwIgYJKoZIhvcNAQkBFhVhcmRhbWFuLnNpbmdoQGFybS5j\n"
   // "b20wHhcNMTgwNjA2MTgzMjU1WhcNMjgwNjAzMTgzMjU1WjCBqDELMAkGA1UEBhMC\n"
   // "VVMxEzARBgNVBAgMCkNhbGlmb3JuaWExETAPBgNVBAcMCFNhbiBKb3NlMQwwCgYD\n"
   // "VQQKDANBUk0xGjAYBgNVBAsMEUlTRy1EYXRhLVNlcnZpY2VzMSEwHwYDVQQDDBht\n"
   // "cXR0Y2xpZW50LmRhdGEubWJlZC5jb20xJDAiBgkqhkiG9w0BCQEWFWFyZGFtYW4u\n"
   // "c2luZ2hAYXJtLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAK0m\n"
   // "RuKbaQQVY8aPUqN+z+nOxbangRpbrKjffhmds7Wqq6+HNaOlxoMs3YgA5DJegNm0\n"
   // "q6iKWrqIo+7jGunFW7M9o/vDEplZeMeJqPz+ojqyLEoj/m6woypQYRIFcLGMPFd6\n"
   // "6/I1ZzCUQw3r8DqEcljdpwtzcQyobIaENJRPSYlgili7UMFZqgnwD0ZdsJoTUcEC\n"
   // "uBAuPYMJdklzO7a/VP1W5Q8YviTDZNyT5jlX/YPVWwXq1nHxbHf9QGd873UD7CP4\n"
   // "2GLoFdkAm4Gv7Oew4eiU+LyjkPFAO7FG/LiNffpmxE0uee9HkqHYxpfvHyvbyCZf\n"
   // "pPHSlEisFsu90CBtBaMCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAR8lOEJkMfqb3\n"
   // "3fbo7t6mQ8RnMCbehgZBsuWyJkQT9/L6ISULUJUBNxUS4/0lOyS+R4hqbQVgGXQV\n"
   // "Xhyb3obOCUv+bFEvxS++6YuoVc7AgoHXIKEP7uXMcPHR/VgIoiFttTgWdkO3I9dU\n"
   // "tuCgpi+QUHGw7DQs7KfHx2tFIywua/UoG/T7f3hGNyACG8b7D8OR8FHrbDR2/48R\n"
   // "VizpF62MQhLC1BEMImV8Vnjgoc3YLPeRFy5aDONKWhrwUGA3zaE22op7KKXaZUkm\n"
   // "JZ07xok/cOB0vZlFVQohUpDIQ7KMGsz7nc4jN/2jYtEMn0sLJQTZe67eyZCJ0o7Q\n"
   // "JhrQdVIGlg==\n"
   // "-----END CERTIFICATE-----\n";

const char *TLS_CLIENT_PKEY = 
   "-----BEGIN RSA PRIVATE KEY-----\n"
   "MIIEpAIBAAKCAQEAzeQmBeX8zbif4FcNOaO3Ord6UkXdUXcnQEiHAwF9di4gfYDf\n"
   "/VNU9c71a161/6ZxMcZErblpEjF+iACZdBKSUJHhX/Q8SHp+SFwnbTs1g6ONqkWt\n"
   "kXefAnAhlbo3mZKY09SbgeDOH02wyj1lYMq/ufPJc9suPaSOyKouwvZJ/oIxQJZ7\n"
   "MoyWz1wRchX3vlErezbE0X6GBx2DhU0UikHCD5oIllJjqbCmCEQjjkewhA6iYb7u\n"
   "s9SpGZ1jCuk/Rts3ADXT5Kx+4QzHYP0zfmrA/WRUdfcMxZOceMZW3MUV5R1RaV95\n"
   "vpwncNrWPf6oD3ntzu39D1xc9lxmEE9njWWD/wIDAQABAoIBAEQCaMzp+2e45Jam\n"
   "KqgvfAbZNH6YtVKDmWOSkYZcp6Sx9+l8+IAlVpsbkdW1o0ifIiTe6TC5Xgu0badf\n"
   "C1FcERisF1Yqxv7p92zPg/aA6FxjWhg0i/DRIqUbhPyAVn1T/0d5oyRzraKJdyxW\n"
   "u3YNgudZr+GlNNEH2mJOVMPxtt9qaXgJF1ubi4jQYAUzgclPOzWN83SWcDBlzlQ6\n"
   "1oy3DchbqwlR6AlTOmSQ/gcQDPuxo/dnDAqfbk++UOl/6PReL/uyWJuYsNpADZ/m\n"
   "891yvaC3cYTD3gtZCJ8941Rg4kqQV9Hmj8eMy7Ma7CCwB1JHg+t5rKPDf49SfHEy\n"
   "lZeoFgECgYEA/wD14Er7MbbRO1AyV5Gp/BCMuR4IhghaPV7DQXdYwflK1yG8Nw+x\n"
   "JlnaflE4f6ty0fLDEdkySW1PPlJ97rfZL+aVOYJIvof9CJC1LdSNAMW7YXiCIDME\n"
   "xohjBrqra/xSvBNxgFRAL5Yy0feqUiax1g00BVwyNYrY/Fn1w1lt/E8CgYEAzrIR\n"
   "kfTHPCzWCYgFOOdj9mh5gGm9181O7OoSfslWDMVnwA9lV5vBxcklT253NT+OAYVe\n"
   "ltOv5CEWUajk8niYxqbwZf9q+7mMJuERG17cfA+tRkl0gzE4gmZgojkiSdhMirai\n"
   "db+LO2cpEQDvZHN8VfigI6Bua1ANxmGYkVmloVECgYBdfSGnsHW1fTvqTfWW6z1x\n"
   "vzOIbr1bPlavJz3Yk4PD4byRcLnLSgJ9d/XF18sp61Z2KFViBsoL53pk8+NdkKOc\n"
   "jyHoap520+5tpHf4LHPhc5zGGGNEoM/AZaJAhlFwjppaf5peHQVHLP7GwdD8u1j5\n"
   "i++lg26Y+KWipuyWSQQsuwKBgQCqRxweZ87yjeVOONZEiKtbRbpCxdn1vFd26WaP\n"
   "OIJBRycvm/9a6x/5Nrob898U6OSt8Bd6na3bfD6My1zBPARwMvr8rCyQb808AlI7\n"
   "gCyXr7qJmt33bkcldf3J5tOOjjiQVeEEq+Wh7Fxye5i74j/d+1LBxtrwYSTX93Le\n"
   "20GN4QKBgQD5OBQ3iGkko0zwi3e3Mh9j9TsPJhQGp4fsTNdNd310DelsJq9eKT7d\n"
   "Fj4Cuaos2mEdrl73UeOl8RLCYab/2/VdEi7ttryma4ox4Q5qqh31b3mT9iDaRK0a\n"
   "yseysS9K+NKd0HI1Wu0GLa35d9u3zfITcL+vCQyYYyrn71CMk9kYlg==\n"
   "-----END RSA PRIVATE KEY-----\n";
   // "-----BEGIN RSA PRIVATE KEY-----\n"
   // "MIIEowIBAAKCAQEArSZG4ptpBBVjxo9So37P6c7FtqeBGlusqN9+GZ2ztaqrr4c1\n"
   // "o6XGgyzdiADkMl6A2bSrqIpauoij7uMa6cVbsz2j+8MSmVl4x4mo/P6iOrIsSiP+\n"
   // "brCjKlBhEgVwsYw8V3rr8jVnMJRDDevwOoRyWN2nC3NxDKhshoQ0lE9JiWCKWLtQ\n"
   // "wVmqCfAPRl2wmhNRwQK4EC49gwl2SXM7tr9U/VblDxi+JMNk3JPmOVf9g9VbBerW\n"
   // "cfFsd/1AZ3zvdQPsI/jYYugV2QCbga/s57Dh6JT4vKOQ8UA7sUb8uI19+mbETS55\n"
   // "70eSodjGl+8fK9vIJl+k8dKUSKwWy73QIG0FowIDAQABAoIBACEBBsny7ZWFrjsO\n"
   // "3qWjamYar70dOJKZntOhphuj37llCsyubR8AXlJqnt9prBWdxdm5gm7h0GF14imK\n"
   // "yHp+z/fea/91M3pff5IpPzjaIHontCF9suXObYuHPrl8p/pvzKCwIYFNhJnR6OYi\n"
   // "buv4iwM9XLXmD0pmYClT0eHjKxUwLWg+r8NKSGKhsnIx2iWz4OtfNG/e4bU0g76l\n"
   // "ascwc5yG61RSF5vVQTv/wMGuKW8P5cKrS5f2+NWRd0FppVH5QLwYIBFKZ1SdodEL\n"
   // "rE7866LBDxUjcpA07Z5lZfgL6hcOQXCXT8haqmVGI3fsUOjK5nyebl8LHt0ofzwy\n"
   // "A72WQOECgYEA1lKHTbpYCLdGgJcdszlSDs+tKKhnRMdaIFvAuVpblRrUPZn4vNZA\n"
   // "WoO5wnCWhPy0gGm/G7s9g6LS+napO7biZttuUdHKBZ3qBMVlMKHM+phVWUE2dUWR\n"
   // "RG3NQDMz6qrwsJ37SyyH4Iz2RmKsJOOr/LRJOlmqj0ftmT32EIech20CgYEAztIR\n"
   // "F2XjeJBYTPHjMJ6vQXuJiN3U3+TKHwYu6gsrtHh31jKIxHGUEZ3vRe/184seIcqg\n"
   // "qF30u7ybtMpdCC6gW8cWxvfcOg6OtemCnOetv3tFE9aHGXbnU1UeqFN42FoxSrYm\n"
   // "9grql8UIW1wz9as2nPoknknsjOhUU/rHLQnQR08CgYBW0zxJOvKrJUSEl7PKhbA+\n"
   // "m9e0nvSnInPapBEhhf+QGjxdcGEab1nG0ZKRuPbhjVa6pxxq6aH0ECSUnznUHTT/\n"
   // "ImpA71J+kAjcQfPKjeHyq3/4FrkvLS26oRkDpzqjGPlFM9s4CyRIzhJ/VT4T+8AT\n"
   // "Mh5wax7zyNnyuO1UqPu6yQKBgHVcRPCXE7aFimXXWQls8pxhAtGUt8h5JqzmMFcF\n"
   // "Eb7uIWp98Jgwr0oz6eQw38tcpTOdrP79mfOyelTkBFixRLPvzKAJZIHZYugdYs2w\n"
   // "tiqTQ8aXFMDBdVEXWzc/brKus4vmw0MZPLf0yeI19xIwHuSDGaZs4nuvFrM0+jM3\n"
   // "f2YHAoGBAIwhd6KNsmKSxIaCcsUyziyMbrGdJePJZDJT1rHelhuW295I+0qYiZiY\n"
   // "Gga7O0GMFFz3gXS0Iy68iLQQkzpYPAOOtzxHXOOKziqzxIzNx6ti5vVnKm6X/ioK\n"
   // "Ecs4dbwKzKhPmo60sEXbMB1mKIUE/uUS5Mo/3P3GKbUjfM99XRdf\n"
   // "-----END RSA PRIVATE KEY-----\n";

static const char * clientID = "mbed-sample";
static const char * userID = "mbedhacks";
static const char * password = "qwer123";
static const char * topic_1 = "mbed-sample";
static const char * topic_2 = "test";

int arrivedcount = 0;

void messageArrived(MessageData& md)
{
    Message &message = md.message;
    printf("Arrived Callback 1 : qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload [%.*s]\r\n", message.payloadlen, (char*)message.payload);
    ++arrivedcount;
}

class CallbackTest
{
    public:
    
    CallbackTest()
        : arrivedcount(0)
    {}
    
    void messageArrived(MessageData& md)
    {
        Message &message = md.message;
        printf("Arrived Callback 2 : qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
        printf("Payload [%.*s]\r\n", message.payloadlen, (char*)message.payload);
        ++arrivedcount;
    }
    
    private:
    
    int arrivedcount;
};

int main(int argc, char* argv[])
{
    pc.baud(115200);
    float version = 0.6;
    CallbackTest testcb;

    mbedtls_printf("HelloMQTT: version is %.2f\r\n", version);

    NetworkInterface* network = easy_connect(true);
    if (!network) {
        return -1;
    }


    MQTTThreadedClient mqtt(network, TLS_CA_PEM, TLS_CLIENT_CERT, TLS_CLIENT_PKEY);

    mbedtls_printf("before anything\n");

    const char* hostname = "a301cl6a149jgy.iot.us-east-2.amazonaws.com";
    // const char* hostname = "10.72.153.34";
    // const char* hostname = "192.168.0.7";    
    int port = 8883;

    MQTTPacket_connectData logindata = MQTTPacket_connectData_initializer;
    logindata.MQTTVersion = 3;
    logindata.clientID.cstring = (char *) clientID;
    //logindata.username.cstring = (char *) userID;
    //logindata.password.cstring = (char *) password;
    
    mqtt.setConnectionParameters(hostname, port, logindata);
    mqtt.addTopicHandler(topic_1, messageArrived);
    mqtt.addTopicHandler(topic_2, &testcb, &CallbackTest::messageArrived);

    mbedtls_printf("before startListener\n");

    // Start the data producer
    // msgSender.start(mbed::callback(&mqtt, &MQTTThreadedClient::startListener));
    mqtt.startListener();
    
    // mbedtls_printf("after startListener\n");

    // int i = 0;
    // while(true)
    // {
    //     PubMessage message;
    //     message.qos = QOS0;
    //     message.id = 123;
        
    //     strcpy(&message.topic[0], topic_1);
    //     sprintf(&message.payload[0], "Testing %d", i);
    //     message.payloadlen = strlen((const char *) &message.payload[0]);
    //     mqtt.publish(message);
        
    //     i++;
    //     //TODO: Nothing here yet ...
    //     Thread::wait(6000);
    // }

}
