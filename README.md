## cyren_as(senderip, sender, fp, [options])

Classify a File pointer (fp) with Cyren's `ctasd`.

**Params**

- senderip `string` - The IP address of the sending server
- sender `string` - The MAIL FROM address
- fp `File` - The mail file
- options `array` - Options array

The following options are available in the **options** array.

- timeout `number` - Timeout in seconds. The default is 5 seconds.
- path `string` - Path to a the ctipd unix socket. The default is `/var/run/ctipd.sock` 
- host `string` - Host of the ctipd server.
- port `number` - Port of the ctipd server.

**Returns**:

* An `array` with keys `spam` (score), `vod` (virus score), `refid` and `rules` on success
* An `array` with key `error` if Cyren reported an error
* `none` on other errors

## cyren_ip(senderip, [options])

Classify an IP address (senderip) with Cyren's `ctipd`.

**Params**

- senderip `string` - The IP address of the sending server
- options `array` - Options array

The following options are available in the **options** array.

- timeout `number` - Timeout in seconds. The default is 5 seconds.
- path `string` - Path to a the ctipd unix socket. The default is `/var/run/ctipd.sock` 
- host `string` - Host of the ctipd server.
- port `number` - Port of the ctipd server.

**Returns**: a `string` with response "permfail", "tempfail" or "accept", or `none` on error
