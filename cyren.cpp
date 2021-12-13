#include <HalonMTA.h>
#include <stdio.h>
#include <cstring>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <algorithm>

HALON_EXPORT
int Halon_version()
{
	return HALONMTA_PLUGIN_VERSION;
}

HALON_EXPORT
bool Halon_init(HalonInitContext* hic)
{
	curl_global_init(CURL_GLOBAL_ALL);
	return true;
}

HALON_EXPORT
void Halon_cleanup()
{
}

struct ctasd_request_data
{
	std::string preamble;
	FILE* fp;
};

size_t curl_string_writer(char *data, size_t size, size_t nmemb, std::string *out)
{
	out->append((const char*)data, size * nmemb);
	return size * nmemb;
}

size_t read_callback(char *dest, size_t size, size_t nmemb, struct ctasd_request_data *d)
{
	if (!d->preamble.empty())
	{
		std::string request = d->preamble.substr(0, size * nmemb);
		d->preamble.erase(0, request.size());
		memcpy(dest, request.c_str(), request.size());
		return request.size();
	}
	size_t x = fread(dest, size, nmemb, d->fp);
	return x;
}

void cyren_as(HalonHSLContext* hhc, HalonHSLArguments* args, HalonHSLValue* ret)
{
	std::string path, address;
	size_t port = 8088, connect_timeout = 0, timeout = 5;

	std::string senderip, mailfrom, senderid;
	size_t rcptcount = 0;

	FILE* fp;
	const HalonHSLValue* a = HalonMTA_hsl_argument_get(args, 0);

	if (a && HalonMTA_hsl_value_type(a) == HALONMTA_HSL_TYPE_FILE)
		HalonMTA_hsl_value_get(a, HALONMTA_HSL_TYPE_FILE, &fp, nullptr);
	else
	{
		HalonHSLValue* x = HalonMTA_hsl_throw(hhc);
 		HalonMTA_hsl_value_set(x, HALONMTA_HSL_TYPE_EXCEPTION, "File argument is not a File object", 0);
		return;
	}

	const HalonHSLValue* b = HalonMTA_hsl_argument_get(args, 1);
	if (b && HalonMTA_hsl_value_type(b) == HALONMTA_HSL_TYPE_ARRAY)
	{
		size_t index = 0;
		HalonHSLValue *k, *v;
		while ((v = HalonMTA_hsl_value_array_get(b, index++, &k)))
		{
			char* string;
			size_t stringl;
			double number;
			if (!HalonMTA_hsl_value_get(k, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
				continue;
			if (std::string(string, stringl) == "address")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
					continue;
				address = std::string(string, stringl);
			}
			if (std::string(string, stringl) == "port")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_NUMBER, &number, nullptr))
					continue;
				port = (size_t)number;
			}
			if (std::string(string, stringl) == "connect_timeout")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_NUMBER, &number, nullptr))
					continue;
				connect_timeout = (size_t)number;
			}
			if (std::string(string, stringl) == "timeout")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_NUMBER, &number, nullptr))
					continue;
				timeout = (size_t)number;
			}
			if (std::string(string, stringl) == "path")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
					continue;
				path = std::string(string, stringl);
			}
			if (std::string(string, stringl) == "senderip")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
					continue;
				senderip = std::string(string, stringl);
			}
			if (std::string(string, stringl) == "mailfrom")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
					continue;
				mailfrom = std::string(string, stringl);
			}
			if (std::string(string, stringl) == "senderid")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
					continue;
				senderid = std::string(string, stringl);
			}
			if (std::string(string, stringl) == "rcptcount")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_NUMBER, &number, nullptr))
					continue;
				rcptcount = (size_t)number;
			}
		}
	}

	CURL *curl = curl_easy_init();

	if (!path.empty())
	{
		curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH,path.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/ctasd/ClassifyMessage_Inline");
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_URL, ("http://" + ((!address.empty()) ? address : "127.0.0.1") + ":" + std::to_string(port) + "/ctasd/ClassifyMessage_Inline").c_str());
	}

	fseek(fp, 0, SEEK_END);
	size_t length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	struct ctasd_request_data d;
	d.fp = fp;
	d.preamble += "x-ctch-pver: 0000001\r\n";
	if (!senderip.empty())
		d.preamble += "x-ctch-senderip: " + senderip + "\r\n";
	if (!mailfrom.empty())
		d.preamble += "x-ctch-mailfrom: " + mailfrom + "\r\n";
	if (!senderid.empty())
		d.preamble += "x-ctch-senderid: " + senderid + "\r\n";
	if (rcptcount)
		d.preamble += "x-ctch-rcptcount: " + std::to_string(rcptcount) + "\r\n";
	d.preamble += "\r\n";

	curl_easy_setopt(curl, CURLOPT_READFUNCTION, ::read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, (void*)&d);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, d.preamble.size() + length);
	if (timeout)
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	if (connect_timeout)
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	std::string data;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_string_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

	CURLcode res = curl_easy_perform(curl);

	long status = 0;
	if (res == CURLE_OK)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		HalonMTA_hsl_value_set(ret, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);
		HalonHSLValue *key, *val;
		HalonMTA_hsl_value_array_add(ret, &key, &val);
		HalonMTA_hsl_value_set(key, HALONMTA_HSL_TYPE_STRING, "error", 0);
		HalonMTA_hsl_value_set(val, HALONMTA_HSL_TYPE_STRING, curl_easy_strerror(res), 0);
		return;
	}

	if (status != 200)
	{
		HalonMTA_hsl_value_set(ret, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);
		HalonHSLValue *key, *val;
		HalonMTA_hsl_value_array_add(ret, &key, &val);
		HalonMTA_hsl_value_set(key, HALONMTA_HSL_TYPE_STRING, "error", 0);
		HalonMTA_hsl_value_set(val, HALONMTA_HSL_TYPE_STRING, data.c_str(), 0);
		return;
	}

	HalonMTA_hsl_value_set(ret, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);
    auto ss = std::stringstream(data);
    for (std::string line; std::getline(ss, line, '\n');)
	{
		line.erase(line.find_last_not_of(" \n\r\t") + 1);
		auto colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		HalonHSLValue *k, *v;
		std::string key = line.substr(0, colon), value;
		std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });
		if (colon + 2 <= line.size())
			value = line.substr(colon + 2);
		if (key == "x-ctch-spam")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "spam", 0);
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::tolower(c); });
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_STRING, value.c_str(), value.size());
			continue;
		}
		if (key == "x-ctch-vod")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "vod", 0);
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::tolower(c); });
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_STRING, value.c_str(), value.size());
			continue;
		}
		if (key == "x-ctch-flags")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "flags", 0);
			double flags = strtod(value.c_str(), nullptr);
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_NUMBER, &flags, 0);
			continue;
		}
		if (key == "x-ctch-error")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "error", 0);
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_STRING, value.c_str(), value.size());
			continue;
		}
		if (key == "x-ctch-refid")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "refid", 0);
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_STRING, value.c_str(), value.size());
			continue;
		}
		if (key == "x-ctch-score")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "score", 0);
			double score = strtod(value.c_str(), nullptr);
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_NUMBER, &score, 0);
			continue;
		}
		if (key == "x-ctch-scorecust")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "score_cust", 0);
			double score_cust = strtod(value.c_str(), nullptr);
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_NUMBER, &score_cust, 0);
			continue;
		}
		if (key == "x-ctch-rules")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "rules", 0);
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);
			HalonHSLValue *k2, *v2;
			double index = 0;
    		auto ss2 = std::stringstream(value);
    		for (std::string rule; std::getline(ss2, rule, ','); ++index)
			{
				if (rule.empty()) continue;
				HalonMTA_hsl_value_array_add(v, &k2, &v2);
				HalonMTA_hsl_value_set(k2, HALONMTA_HSL_TYPE_NUMBER, &index, 0);
				HalonMTA_hsl_value_set(v2, HALONMTA_HSL_TYPE_STRING, rule.c_str(), rule.size());
			}
			continue;
		}
		if (key == "x-ctch-maliciouscategory")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "malicious_category", 0);
			double malicious_category = strtod(value.c_str(), nullptr);
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_NUMBER, &malicious_category, 0);
			continue;
		}
	}
}

void cyren_ip(HalonHSLContext* hhc, HalonHSLArguments* args, HalonHSLValue* ret)
{
	std::string path, address;
	size_t port = 8080, connect_timeout = 0, timeout = 5;

	std::string ip;
	char* ip_;
	size_t ipl;
	const HalonHSLValue* a = HalonMTA_hsl_argument_get(args, 0);
	if (a && HalonMTA_hsl_value_type(a) == HALONMTA_HSL_TYPE_STRING)
	{
		HalonMTA_hsl_value_get(a, HALONMTA_HSL_TYPE_STRING, &ip_, &ipl);
		ip = std::string(ip_, ipl);
	}
	else
	{
		HalonHSLValue* x = HalonMTA_hsl_throw(hhc);
		HalonMTA_hsl_value_set(x, HALONMTA_HSL_TYPE_EXCEPTION, "Not a valid IP", 0);
		return;
	}

	const HalonHSLValue* b = HalonMTA_hsl_argument_get(args, 1);
	if (b && HalonMTA_hsl_value_type(b) == HALONMTA_HSL_TYPE_ARRAY)
	{
		size_t index = 0;
		HalonHSLValue *k, *v;
		while ((v = HalonMTA_hsl_value_array_get(b, index++, &k)))
		{
			char* string;
			size_t stringl;
			double number;
			if (!HalonMTA_hsl_value_get(k, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
				continue;
			if (std::string(string, stringl) == "address")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
					continue;
				address = std::string(string, stringl);
			}
			if (std::string(string, stringl) == "port")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_NUMBER, &number, nullptr))
					continue;
				port = (size_t)number;
			}
			if (std::string(string, stringl) == "connect_timeout")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_NUMBER, &number, nullptr))
					continue;
				connect_timeout = (size_t)number;
			}
			if (std::string(string, stringl) == "timeout")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_NUMBER, &number, nullptr))
					continue;
				timeout = (size_t)number;
			}
			if (std::string(string, stringl) == "path")
			{
				if (!HalonMTA_hsl_value_get(v, HALONMTA_HSL_TYPE_STRING, &string, &stringl))
					continue;
				path = std::string(string, stringl);
			}
		}
	}

	CURL *curl = curl_easy_init();

	if (!path.empty())
	{
		curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, path.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/ctIPd/iprep");
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_URL, ("http://" + ((!address.empty()) ? address : "127.0.0.1") + ":" + std::to_string(port) + "/ctIPd/iprep").c_str());
	}

	std::string request;
	request += "x-ctch-request-id: NAN\r\n";
	request += "x-ctch-request-type: classifyip\r\n";
	request += "x-ctch-pver: 1.0\r\n";
	request += "\r\n";
	request += "x-ctch-ip: " + ip;

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	if (timeout)
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	if (connect_timeout)
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	std::string data;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_string_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

	CURLcode res = curl_easy_perform(curl);

	long status = 0;
	if (res == CURLE_OK)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		HalonMTA_hsl_value_set(ret, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);
		HalonHSLValue *key, *val;
		HalonMTA_hsl_value_array_add(ret, &key, &val);
		HalonMTA_hsl_value_set(key, HALONMTA_HSL_TYPE_STRING, "error", 0);
		HalonMTA_hsl_value_set(val, HALONMTA_HSL_TYPE_STRING, curl_easy_strerror(res), 0);
		return;
	}

	if (status != 200)
	{
		HalonMTA_hsl_value_set(ret, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);
		HalonHSLValue *key, *val;
		HalonMTA_hsl_value_array_add(ret, &key, &val);
		HalonMTA_hsl_value_set(key, HALONMTA_HSL_TYPE_STRING, "error", 0);
		HalonMTA_hsl_value_set(val, HALONMTA_HSL_TYPE_STRING, data.c_str(), 0);
		return;
	}

	HalonMTA_hsl_value_set(ret, HALONMTA_HSL_TYPE_ARRAY, nullptr, 0);
    auto ss = std::stringstream(data);
    for (std::string line; std::getline(ss, line, '\n');)
	{
		line.erase(line.find_last_not_of(" \n\r\t") + 1);
		auto colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		HalonHSLValue *k, *v;
		std::string key = line.substr(0, colon), value;
		std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });
		if (colon + 1 <= line.size())
			value = line.substr(colon + 1);
		if (key == "x-ctch-dm-action")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "action", 0);
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::tolower(c); });
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_STRING, value.c_str(), value.size());
			continue;
		}
		if (key == "x-ctch-refid:tid")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "refid", 0);
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::tolower(c); });
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_STRING, value.c_str(), value.size());
			continue;
		}
		if (key == "x-ctch-error")
		{
			HalonMTA_hsl_value_array_add(ret, &k, &v);
			HalonMTA_hsl_value_set(k, HALONMTA_HSL_TYPE_STRING, "error", 0);
			HalonMTA_hsl_value_set(v, HALONMTA_HSL_TYPE_STRING, value.c_str(), value.size());
			continue;
		}
	}
}

HALON_EXPORT
bool Halon_hsl_register(HalonHSLRegisterContext* ptr)
{
	HalonMTA_hsl_register_function(ptr, "cyren_as", &cyren_as);
	HalonMTA_hsl_register_function(ptr, "cyren_ip", &cyren_ip);
	return true;
}