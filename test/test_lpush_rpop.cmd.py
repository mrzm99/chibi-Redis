import socket

# サーバーの設定
HOST = '127.0.0.1'
PORT = 6333

def to_resp_array(cmd_list):
    """文字列のリストをRESPのArrayフォーマットに変換する"""
    resp = f"*{len(cmd_list)}\r\n"
    for arg in cmd_list:
        arg_bytes = arg.encode('utf-8')
        resp += f"${len(arg_bytes)}\r\n{arg}\r\n"
    return resp

def send_command(sock, resp_payload):
    sock.sendall(resp_payload.encode('utf-8'))
    try:
        return sock.recv(1024).decode('utf-8')
    except ConnectionResetError:
        return ""

def run_test(name, sock, cmd_list, expected):
    human_readable = " ".join(cmd_list)
    print(f"[{name}] 送信: {human_readable}")
    
    actual = send_command(sock, to_resp_array(cmd_list))
    
    if actual == expected:
        print(f"  ✅ PASS (応答: {repr(actual)})")
    else:
        print(f"  ❌ FAIL\n     期待: {repr(expected)}\n     実際: {repr(actual)}")
    print("-" * 40)

def main():
    print("=== 自作Redis RESPパケット自動テスト (List型追加版) ===")
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            
            # --- 環境の初期化 (前回のテストデータが残っている場合を考慮) ---
            send_command(s, to_resp_array(["DEL", "mylist", "mystring", "tmplist"]))

            print("\n--- 1. LPUSH コマンドのテスト ---")
            # 新規リストの作成と1件追加
            run_test("TEST 1-1", s, ["LPUSH", "mylist", "world"], ":1\r\n")
            # 既存リストへの1件追加 -> [hello, world] になる
            run_test("TEST 1-2", s, ["LPUSH", "mylist", "hello"], ":2\r\n")
            # 複数要素の同時追加 -> [B, A, hello, world] になる
            run_test("TEST 1-3", s, ["LPUSH", "mylist", "A", "B"], ":4\r\n")

            print("\n--- 2. RPOP コマンドのテスト ---")
            # 現在のリスト: [B, A, hello, world]
            # 一番右(=一番古い要素) 'world' が取り出されるか
            run_test("TEST 2-1", s, ["RPOP", "mylist"], "$5\r\nworld\r\n")
            # 続いて 'hello' が取り出されるか
            run_test("TEST 2-2", s, ["RPOP", "mylist"], "$5\r\nhello\r\n")
            # 残りを全て RPOP ('A' -> 'B' の順)
            run_test("TEST 2-3", s, ["RPOP", "mylist"], "$1\r\nA\r\n")
            run_test("TEST 2-4", s, ["RPOP", "mylist"], "$1\r\nB\r\n")
            
            # 空になった状態での RPOP (nil が返るべき)
            run_test("TEST 2-5", s, ["RPOP", "mylist"], "$-1\r\n")
            # 存在しないキーに対する RPOP (nil が返るべき)
            run_test("TEST 2-6", s, ["RPOP", "unknown_list"], "$-1\r\n")

            print("\n--- 3. WRONGTYPE (型エラー) のテスト ---")
            # String型キーの作成
            run_test("TEST 3-1", s, ["SET", "mystring", "hoge"], "+OK\r\n")
            # String型に対して LPUSH (エラーになるべき)
            run_test("TEST 3-2", s, ["LPUSH", "mystring", "fuga"], "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n")
            # String型に対して RPOP (エラーになるべき)
            run_test("TEST 3-3", s, ["RPOP", "mystring"], "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n")
            # List型に対して GET (エラーになるべき)
            run_test("TEST 3-4", s, ["LPUSH", "tmplist", "val"], ":1\r\n")
            run_test("TEST 3-5", s, ["GET", "tmplist"], "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n")

            print("\n--- 4. 空リストの自動削除仕様のテスト ---")
            # 要素を1つ追加し、キーが存在することを確認
            run_test("TEST 4-1", s, ["EXISTS", "tmplist"], ":1\r\n")
            # 要素を取り出して空にする
            run_test("TEST 4-2", s, ["RPOP", "tmplist"], "$3\r\nval\r\n")
            # 空になったので、キー自体が kv_store から消滅しているはず
            run_test("TEST 4-3", s, ["EXISTS", "tmplist"], ":0\r\n")

    except ConnectionRefusedError:
        print("❌ エラー: サーバーに接続できません。C++のサーバーが起動しているか確認してください。")

if __name__ == "__main__":
    main()
