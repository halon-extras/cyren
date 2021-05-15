## cyren_as(senderip, sender, fp)

Classify a File pointer (fp) with Cyren's `ctasd`.

**Params**

- senderip `string` - The IP address of the sending server
- sender `string` - The MAIL FROM address
- fp `File` - the mail file

**Returns**:

* An `array` with keys `spam` (score), `vod` (virus score), `refid` and `rules` on success
* An `array` with key `error` if Cyren reported an error
* `none` on other errors

## cyren_ip(senderip)

Classify an IP address (senderip) with Cyren's `ctipd`.

**Params**

- senderip `string` - The IP address of the sending server

**Returns**: a `string` with response "permfail", "tempfail" or "accept", or `none` on error
