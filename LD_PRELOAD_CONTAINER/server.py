from http.server import HTTPServer, BaseHTTPRequestHandler
import json
from datetime import datetime

class ExecHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        if self.path == '/api/exec':
            content_length = int(self.headers['Content-Length'])
            body = self.rfile.read(content_length)
            
            try:
                data = json.loads(body)
                timestamp = datetime.now().isoformat()
                print(f"[{timestamp}] EXEC captured:")
                print(f"  Filename: {data.get('filename')}")
                print(f"  Args: {data.get('args')}")
                print(f"  Hostname: {data.get('hostname')}")
                print(f"  PID: {data.get('pid')}")
                print("-" * 60)
                
                self.send_response(200)
                self.end_headers()
                self.wfile.write(b'OK')
            except Exception as e:
                print(f"Error: {e}")
                self.send_response(500)
                self.end_headers()
    
    def log_message(self, format, *args):
        pass  # Désactiver les logs HTTP standards

if __name__ == '__main__':
    server = HTTPServer(('0.0.0.0', 80), ExecHandler)
    print("Listening on port 80...")
    server.serve_forever()
