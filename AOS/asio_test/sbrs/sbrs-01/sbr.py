from google.appengine.api import memcache
import urllib2,re,threading,time

def getSBRS(ip, mask, pos, row, retry):
    page = getHTML(ip, mask, pos, row, retry)
    if page is None:
        return None
    
    total = getTotalCount(page)
    csv = getCSV2(page)
    ret = "+%d record(s) found [%s/%s]\r\n%s" % (total,pos,row,csv)
    return ret

def getHTML(ip, mask, pos, row, retry):
    if int(mask) < 16:
        mask = '16';
    if int(mask) > 32:
        mask = '32';
    if int(row) < 1:
        row = '1';
    if int(row) > 50:
        row = '50';

    url = 'http://www.senderbase.org/senderbase_queries/detailip?search_string=%s/%s&dnext_set=%s&max_rows=%s' %(ip, mask, pos, row)

    req = urllib2.Request(url=url)
    for t in xrange(retry):
        try:
            doc = urllib2.urlopen(req)
            return doc.read()
        except:
            continue
    return None

def getSBR(a_,b_,i_,tries):
    # return result csv string, on error return None
    key = "%s.%s_%s" %(a_,b_,i_)
    data = memcache.get(key)
    if data:
        return "0 cache hit\r\n" + data
    
    page = getPage(a_,b_,i_,tries)
    if page is None:
        return None
    
    total = getTotalCount(page)
    csv = getCSV2(page)
    ret = "[total:%d]\r\n%s" %(total,csv)
    memcache.set(key,ret,time=5) #expire time
    return "0 cache miss\r\n" + ret
    
def getPage(a_,b_,i_,tries):
    url = 'http://www.senderbase.org/senderbase_queries/detailip?search_string=%s.%s.1.1/16&dnext_set=%s' %(a_,b_,i_)

    req = urllib2.Request(url=url)
    for t in xrange(tries):
        try:
            doc = urllib2.urlopen(req)
            return doc.read()
        except:
            continue
    return None

def getTotalCount(page):
    total = 0
    pattern = re.compile('.*out of (\d+)',re.DOTALL)
    m = pattern.match(page)
    if m: total = int(m.group(1))
    return total

def getCSV(page):
    ret = ''
    get_ip = re.compile('.*?>(\d+\.\d+\.\d+\.\d+)<')
    get_sbr_good = re.compile('.*?>(Good)<')
    get_sbr_neutral = re.compile('.*?>(Neutral)<')
    get_sbr_poor = re.compile('.*?>(Poor)<')

    for line in page.splitlines():

        m = get_ip.match(line)
        if m:
            ret += m.group(1) + ','
            continue

        m = get_sbr_good.match(line)
        if m:
            ret += 'Good\r\n'
            continue

        m = get_sbr_neutral.match(line)
        if m:
            ret += 'Neutral\r\n'
            continue

        m = get_sbr_poor.match(line)
        if m:
            ret += 'Poor\r\n'
            continue
    return ret
    
def getCSV2(page):
    ret = ''
    reIP = re.compile('.*?>(\d+\.\d+\.\d+\.\d+)<')
    reSBRS = re.compile('.*?>(Good|Neutral|Poor)<', re.IGNORECASE)
    
    lines = page.splitlines();
    n_line = len(lines);
    
    #print 'n_line=%d\r\n' %(n_line);
    i = 0;
    while( i < n_line ):
        m = re.match(r'.*?>(\d+\.\d+\.\d+\.\d+)<', lines[i], re.IGNORECASE);
        if m:
            ip = m.group(1);
            hostname = '';
            dns_match = '';
            daily = '';
            monthly = '';
            dnsbl = '';
            sbrs = '';

            #m1 = re.match(r'([\w\.]+)<\/div>', lines[i+3], re.IGNORECASE);
            #if m1:
            #    hostname = m1.group(1);
                
            splits = lines[i+3].split('<');
            hostname = splits[0].strip();
            m2 = re.match(r'.*?>([\w\.]+)<\/div>', lines[i+5], re.IGNORECASE);
            if m2:
                dns_match = m2.group(1);
            m3 = re.match(r'.*?>([\w\.]+)<\/div>', lines[i+6], re.IGNORECASE);
            if m3:
                daily = m3.group(1);                
            m4 = re.match(r'.*?>([\w\.]+)<\/div>', lines[i+7], re.IGNORECASE);
            if m4:
                monthly = m4.group(1);
            m5 = re.match(r'.*?>([\w\.]+)<\/div>', lines[i+8], re.IGNORECASE);
            if m5:
                dnsbl = m5.group(1);
            m6 = re.match(r'.*?>(Good|Neutral|Poor)<', lines[i+9], re.IGNORECASE);
            if m6:
                sbrs = m6.group(1);
                
            if ( len(sbrs) > 0 ):
                ret += '%s,%s,%s,%s,%s,%s,%s\r\n' %(ip, hostname, dns_match, daily, monthly, dnsbl, sbrs);
            i += 10;
            
        i += 1;

    return ret
