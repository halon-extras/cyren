# Cyren client plugin

## Installation

Follow the [instructions](https://docs.halon.io/manual/comp_install.html#installation) in our manual to add our package repository and then run the below command.

### Ubuntu

```
apt-get install halon-extras-cyren
```

### RHEL

```
yum install halon-extras-cyren
```

## Exported functions

### cyren_as(fp, [options])

Classify a File pointer (fp) with Cyren's `ctasd`.

**Params**

- fp `File` - The mail file
- options `array` - Options array

The following options are available in the **options** array.

- path `string` - Path to a the ctasd unix socket.
- address `string` - Host of the ctasd server. The default is `localhost`
- port `number` - Port of the ctasd server. The default is `8088`
- connect_timeout `number` - The connect timeout. The default is no timeout.
- timeout `number` - The timeout. The default is 5 seconds.
- senderip `string` - The IP address of the sending server (optional)
- mailfrom `string` - The MAIL FROM address (optional)
- senderid `string` - The sender ID (optional)
- rcptcount `number` - Number of recipients (optional)

**Returns**

* An `array` with keys `spam` (score), `vod` (virus score), `refid` and `rules` on success (etc.)
* An `array` with key `error` on errors

### cyren_ip(senderip, [options])

Classify an IP address (senderip) with Cyren's `ctipd`.

**Params**

- senderip `string` - The IP address of the sending server
- options `array` - Options array

The following options are available in the **options** array.

- path `string` - Path to a the ctipd unix socket.
- address `string` - Host of the ctipd server. The default is `localhost`
- port `number` - Port of the ctipd server. The default is `8080`
- connect_timeout `number` - The connect timeout. The default is no timeout.
- timeout `number` - Timeout in seconds. The default is 5 seconds.

**Returns**

* An `array` with keys `action` (recommened action) and `refid` on success (etc.)
* An `array` with key `error` on errors
