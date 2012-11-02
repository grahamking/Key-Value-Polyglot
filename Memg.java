import java.io.*;
import java.net.*;
import java.util.*;

public class Memg {

	public static final Map<String, String> cache = new HashMap<String, String>();
	
	public static void main(String[] args) {	
		ServerSocket sock = null;
		try {
			sock = new ServerSocket(11211);
		} catch (IOException ex) {
			ex.printStackTrace();
		}
		if (sock != null) {
			if (find("--single", args)) {
				try {
					final Socket conn = sock.accept();
					handleConnection(conn);
				} catch (IOException ex) {
					ex.printStackTrace();
				}
			} else {
				while (true) {
					try {
						final Socket conn = sock.accept();
						Thread t = new Thread() {
							@Override
							public void run() {
								try {
									handleConnection(conn);
								} catch (IOException ex) {
									ex.printStackTrace();
								}
							}
						};
						t.start();
					} catch (IOException ex) {
						ex.printStackTrace();
					}
				}
			}
		}
	}

	private static boolean find(String key, String[] arr) {
		for (String s : arr) {
			if (s.equals(key)) {
				return true;
			}
		}
		return false;
	}

	private static void handleConnection(Socket conn) throws IOException {
		BufferedReader in = new BufferedReader(new InputStreamReader(conn.getInputStream()));
		PrintWriter out = new PrintWriter(conn.getOutputStream(), false);	
		while(true) {
			String line = in.readLine();
			if(line.equals(""))
				break;
			String[] parts = line.split(" ");
			String cmd = parts[0];
			if(cmd.equals("get")) {
				String key = parts[1];
				String val;
				if(cache.containsKey(key)) {
					val = cache.get(key);
					out.printf("VALUE %s 0 %d\r\n", key, val.length());
					out.printf("%s\r\n",val);
				}
				out.printf("END\r\n");
			} else if(cmd.equals("set")) {
				String key = parts[1];
				int length = Integer.parseInt(parts[4]);
				char[] buf = new char[length + 2]; //This is not 100% correct for handling unicode bytes, but it's fine here for the benchmark test
				int index = 0;
				while (index < buf.length) {
					int len = in.read(buf, index, buf.length - index);
					if (len == -1)
						break;
					index += len;
				}
				String val = new String(buf, 0, length);
				cache.put(key, val);
				out.printf("STORED\r\n");
			} else {
				System.out.println("Unknown cmd: " + cmd);
				break;
			}
			out.flush();
		}
	}
}

