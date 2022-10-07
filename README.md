# Cyren anti-spam client plugin

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

* An `array` with key `error` on errors
* An `array` with scan results

| Key | | Type | Value/Examples
|-----|-|------|----------------
| spam | | string | ``confirmed``, ``bulk``, ``suspect``, ``unknown``, ``nonspam`` or ``valid-bulk``
| vod  | |string  | ``virus``, ``high``, ``medium``, ``unknown`` or ``nonvirus``
| flags | | number         | (internal)
| refid | | string         | `str=0001.0A682F1B.61FD04EF.0024,ss=1,re=0.000,recu=0.000,reip=0.000,cl=1,cld=1,fgs=0`
| score | | number         | the final spam score
| score_cust | | number         | the localview custom rules
| rules | | array[]string   | matched localview rules
| malicious_category| | number |  ``0`` (No phishing or malware URL was detected), ``1`` (A malware URL was detected), ``2`` (A phishing URL was detected)
| virus |          | array[] |
| | type         | string | eg. `virus`
| | accuracy         | string | eg. `exact`
| | name         | string | name of virus found

**Example (EOD)**

```
import { cyren_as } from "extras://cyren";

echo cyren_as($arguments["mail"]->toFile());
```

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

* An `array` with key `error` on errors
* An `array` with scan results

| Key | | Type | Value/Examples
|-----|-|------|----------------
| action | | string | ``accept``, ``tempfail`` or ``permfail``
| refid | | string    | `str=0001.0A682F1B.61FD04EF.0024,ss=1,re=0.000,recu=0.000,reip=0.000,cl=1,cld=1,fgs=0`

**Example (Connect)**

```
import { cyren_ip } from "extras://cyren";

echo cyren_ip($arguments["remoteip"]);
```