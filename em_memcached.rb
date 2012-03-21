require 'eventmachine'

$cache = {}

EM.run do
  EM.start_server('127.0.0.1', 11211) do |c|
    def c.receive_data(data)
      close_connection if data == 'quit\r\n' or data.to_s.empty?
      cmd = data[0..2]

      if cmd == 'get'
        key = data.split[1]
        buffer = []
        if val = $cache[key]
          buffer << "VALUE #{key} 0 #{val.size}\r\n"
          buffer << "#{val}\r\n"
        end
        buffer << "END\r\n"
        send_data buffer.join('')
      elsif cmd == 'set'
        header, val = data.split("\r\n")
        header = header.split
        $cache[header[1]] = val[0...header[4].to_i] 
        send_data "STORED\r\n"
      end
    end
  end
end
