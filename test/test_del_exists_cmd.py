import socket

# サーバーの設定 (C++サーバーのポートに合わせて変更してください)
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
    # QUIT実行後など、接続が切れた場合は空文字が返る可能性があるため例外処理を入れると安全ですが、
    # 今回はシンプルなテストなのでそのままrecvします
    try:
        response = sock.recv(1024).decode('utf-8')
        return response
    except ConnectionResetError:
        return ""

def run_test(name, sock, cmd_list, expected):
    human_readable = " ".join(cmd_list)
    print(f"[{name}] 送信: {human_readable}")
    
    resp_payload = to_resp_array(cmd_list)
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
            
            # --- 既存のSET/GETテスト ---
            run_test("TEST 1", s, ["SET", "name", "Alice"], "+OK\r\n")
            run_test("TEST 2", s, ["SET", "city", "Tokyo"], "+OK\r\n")
            
            # --- EXISTSのテスト ---
            print("\n--- EXISTS コマンドのテスト ---")
            # 単一の存在するキー
            run_test("TEST 3-1", s, ["EXISTS", "name"], ":1\r\n")
            # 存在しないキー
            run_test("TEST 3-2", s, ["EXISTS", "unknown"], ":0\r\n")
            # 複数キー (存在するキー2つ + 存在しないキー1つ = 2が返るべき)
            run_test("TEST 3-3", s, ["EXISTS", "name", "city", "unknown"], ":2\r\n")

            # --- DELのテスト ---
            print("\n--- DEL コマンドのテスト ---")
            # 存在しないキーの削除 (0が返るべき)
            run_test("TEST 4-1", s, ["DEL", "unknown"], ":0\r\n")
            # 単一の存在するキーの削除
            run_test("TEST 4-2", s, ["DEL", "name"], ":1\r\n")
            # 削除されたことの確認 (EXISTSで0になるか)
            run_test("TEST 4-3", s, ["EXISTS", "name"], ":0\r\n")
            
            # もう一度データをセット
            run_test("TEST 4-4", s, ["SET", "key1", "val1"], "+OK\r\n")
            run_test("TEST 4-5", s, ["SET", "key2", "val2"], "+OK\r\n")
            # 複数キーの削除 (存在するキー2つ + 存在しないキー1つ = 2が返るべき)
            run_test("TEST 4-6", s, ["DEL", "key1", "key2", "unknown"], ":2\r\n")

            print("-" * 40)

    except ConnectionRefusedError:
        print("❌ エラー: サーバーに接続できません。C++のサーバーが起動しているか確認してください。")

if __name__ == "__main__":
    main()
