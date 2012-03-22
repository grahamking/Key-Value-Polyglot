import java.nio._
import java.io._
import java.util.Scanner
import java.nio.channels._
import java.net._
import java.util.concurrent._
import scala.collection.mutable.HashMap

val executor = Executors.newCachedThreadPool()
val serverSocket = new ServerSocket(11211)
val cache = new HashMap[String, String]()

def printStackTrace(expression: => Unit) {
	try {
		expression
	} catch {
	  case throwable => throwable.printStackTrace()
	}
}

def handleConnection(socket: Socket) {
  val in = socket.getInputStream()
  val out = new PrintWriter(socket.getOutputStream(), true)
  var continue: Boolean = true
  while(continue) {
    val line = new String(Iterator
      .continually(in.read())
      .takeWhile(_ != -1)
      .map(_.toByte)
      .takeWhile(_ != '\r')
      .toArray
    )
    in.read() // Consume LF.
    if (line == "") {
      continue = false
    } else {
      println("Received line: %s.".format(line))
      line.split(" ").toList match {
        case "get" :: key :: rest => {
          cache.get(key).foreach{value =>
            out.print("VALUE %s 0 %d\r\n".format(key, value.length))
            out.print("%s\r\n".format(value))
          }
          println("Sending end for key %s.".format(key))
          out.printf("END\r\n")
        }
        case "set" :: key :: flags :: exptime :: byteLength :: rest => {
          val length = byteLength.toInt
          val bytes = Array.ofDim[Byte](length + 2)
          in.read(bytes)
          val value = new String(bytes).substring(0, length)
          cache.put(key, value)
          out.printf("STORED\r\n")
        }
        case "quit" :: rest => continue = false
        case other => {
          println("Received unknown request: %s".format(other))
        }
      }
    }
  }
}

if (args.contains("--single")) {
	printStackTrace {
		val socket = serverSocket.accept()
		handleConnection(socket)
	}
} else {
	while (true) {
	  printStackTrace {
	    val socket = serverSocket.accept()
			executor.submit(new Runnable(){
			  def run() {
			    printStackTrace {
			      handleConnection(socket)
			    }
			  }
			})
		}
	}
}
