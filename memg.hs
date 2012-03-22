{-# LANGUAGE OverloadedStrings #-}

import Network
import Control.Applicative
import Control.Concurrent (forkIO)
import Control.Monad (forever)
import Data.Attoparsec.ByteString as A
import Data.Attoparsec.ByteString.Char8 (endOfLine, char, isSpace_w8, decimal, signed)
import Data.Attoparsec.Enumerator
import Data.ByteString.Char8 as C
import Data.Enumerator as E
import System.IO
import System.Environment
import qualified Data.ByteString as B
import qualified Data.Enumerator.List as EL
import qualified Data.Enumerator.Binary as EB
import qualified Data.HashTable.IO as H

type HashTable = H.BasicHashTable Key Value
type Key       = ByteString
type Value     = ByteString

data Command = Get Key | Set Key Value deriving Show

command :: Parser Command
command = (word >>= mkCommand) <* endOfLine where
    mkCommand "get" = Get <$> word
    mkCommand "set" = Set <$> word <* extra <*> value
    mkCommand cmd   = fail $ "invalid command: " ++ C.unpack cmd

    word   = takeWhile1 (not.isSpace_w8) <* optional space
    value  = decimal >>= (endOfLine *>) . A.take
    extra  = number >> number >> return ()
    number = signed decimal >> space
    space  = char ' '

serve :: Socket -> HashTable -> IO ()
serve socket table = acceptConn where
    acceptConn = do
        (handle,_,_) <- accept socket
        hSetBuffering handle LineBuffering
        _ <- forkIO $ serveClient handle
        return ()

    serveClient handle = exec $ commands $$ respond where
        exec i   = run i >> return ()
        commands = EB.enumHandle 1024 handle $= E.sequence (iterParser command)
        respond  = EL.concatMapM response =$ EB.iterHandle handle

        response (Get key) = do
            val <- lookup key
            case val of
                Just val -> do
                    let len = C.pack . show . B.length $ val
                    return ["VALUE ", key, " 0 ", len, "\n", val, "\nEND\n"]
                Nothing -> return ["END\n"]

        response (Set key value) = insert key value >> return ["STORED\n"]

    insert = H.insert table
    lookup = H.lookup table

main :: IO ()
main = withSocketsDo $ do
    socket <- listenOn (PortNumber 11211)
    table  <- H.new
    args   <- getArgs
    case args of
        ["--single"] -> serve socket table
        _            -> forever $ serve socket table
