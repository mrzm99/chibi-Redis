import socket

# サーバーの設定 (C++サーバーのポートに合わせて変更してください)
HOST = '127.0.0.1'
PORT = 6333

def to_resp_array(cmd_list):
    """
    文字列のリストをRESPのArrayフォーマットに変換する
    例: ["SET", "name", "Alice"] 
    -> "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$5\r\nAlice\r\n"
    """
    resp = f"*{len(cmd_list)}\r\n"
    for arg in cmd_list:
        # 文字列のバイト長を取得（マルチバイト文字が混ざっても正確にするため）
        arg_bytes = arg.encode('utf-8')
        resp += f"${len(arg_bytes)}\r\n{arg}\r\n"
    return resp

def send_command(sock, resp_payload):
    # すでに構築済みのRESPパケットをそのまま送信
    sock.sendall(resp_payload.encode('utf-8'))
    # 応答を受信
    response = sock.recv(1024).decode('utf-8')
    return response

def run_test(name, sock, cmd_list, expected):
    # コンソール表示用（人間が読みやすい形式）
    human_readable = " ".join(cmd_list)
    print(f"[{name}] 送信: {human_readable}")
    
    # RESPペイロードの構築
    resp_payload = to_resp_array(cmd_list)
    # デバッグ用に送信する生のパケットを表示したい場合は以下のコメントアウトを外してください
    # print(f"  [Raw Payload]: {repr(resp_payload)}")
    
    # 実行と結果取得
    actual = send_command(sock, resp_payload)
    
    if actual == expected:
        print(f"  ✅ PASS (応答: {repr(actual)})")
    else:
        print(f"  ❌ FAIL\n     期待: {repr(expected)}\n     実際: {repr(actual)}")
    print("-" * 40)

def main():
    print("=== 自作Redis RESPパケット自動テスト開始 ===")
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            
            # リスト形式で渡すことで、内部で自動的にRESP Arrayに変換されます
            
            # 1. 基本的なSETとGET
            run_test("TEST 1-1", s, ["SET", "name", "Alice"], "+OK\r\n")
            run_test("TEST 1-2", s, ["GET", "name"], "$5\r\nAlice\r\n")
            
            # 2. 存在しないキー
            run_test("TEST 2-1", s, ["GET", "unknown"], "$-1\r\n")
            
            # 3. 値の上書き
            run_test("TEST 3-1", s, ["SET", "name", "Bob"], "+OK\r\n")
            run_test("TEST 3-2", s, ["GET", "name"], "$3\r\nBob\r\n")
            
            # 4. 引数エラーのハンドリング
            # ※エラーメッセージの文字列は実装に合わせて適宜書き換えてください
            actual_err = send_command(s, to_resp_array(["SET", "age"]))
            if actual_err.startswith("-ERR"):
                print("[TEST 4-1] 送信: SET age\n  ✅ PASS (引数エラーを捕捉)")
            else:
                print(f"[TEST 4-1] 送信: SET age\n  ❌ FAIL (エラー応答が不正: {repr(actual_err)})")
            print("-" * 40)

            # 5. コマンド名の大文字小文字の区別テスト
            run_test("TEST 5-1", s, ["sEt", "city", "Tokyo"], "+OK\r\n")
            run_test("TEST 5-2", s, ["gEt", "city"], "$5\r\nTokyo\r\n")

    except ConnectionRefusedError:
        print("❌ エラー: サーバーに接続できません。C++のサーバーが起動しているか確認してください。")

if __name__ == "__main__":
    main()
