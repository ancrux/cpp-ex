from google.appengine.ext import webapp
from google.appengine.ext.webapp import util
import sbr

class GetSBRSHandler(webapp.RequestHandler):
    def get(self):
        ip = self.request.get('ip',default_value='8.0.1.1')
        mask = self.request.get('mask',default_value='16')
        pos = self.request.get('pos',default_value='0')
        row = self.request.get('row',default_value='50')
        retry = self.request.get('retry',default_value='3')
        
        result = sbr.getSBRS(ip, mask, pos, row, int(retry))
        
        if result is None:
            result = '-1 fetch page failed\r\n'
            
        self.response.headers['Content-Type'] = 'text/plain'
        self.response.out.write(result)

class GetSBRHandler(webapp.RequestHandler):
    def get(self):
        a = self.request.get('a',default_value='1')
        b = self.request.get('b',default_value='1')
        i = self.request.get('i',default_value='0')
        t = self.request.get('t',default_value='3')
        
        result = sbr.getSBR(a,b,i,int(t))
        
        if result is None:
            result = '-1 get page fail\r\n'
            
        self.response.headers['Content-Type'] = 'text/plain'
        self.response.out.write(result)
           
def main():
    application = webapp.WSGIApplication(
                                        [('/get', GetSBRSHandler)],
                                         debug=True)
    util.run_wsgi_app(application)


if __name__ == '__main__':
    main()
