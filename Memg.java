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
		Scanner in = new Scanner(conn.getInputStream());
		PrintWriter out = new PrintWriter(conn.getOutputStream(), true);	
		while(true) {
			String line = in.nextLine();
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
				byte[] bs = new byte[length+2];
				int bytesRead=0;
				while(bytesRead < length+2) {
					bs[bytesRead] = (byte)conn.getInputStream().read();
					bytesRead++;
				}
				String val = new String(bs);
				cache.put(key, val.substring(0, length));
				out.printf("STORED\r\n");
			}
		}
	}
}

