"""
generate, with stockfish, perft results for specified depth for all fen stored in given file with each fen stored in separate line and save results in output file.
output file format:
<fen1>;<depth>;<perft_result1>
<fen2>;<depth>;<perft_result2>
input file format:
<fen1>
<fen2>
run with python generate_perft_results.py <stockfish_executable_path> <file_with_fens_path> <output_file_path> <depth>
"""






def main():
    import sys
    import subprocess
    from tqdm import tqdm
    stockfish_path = sys.argv[1]
    depth = sys.argv[4]
    stockfish_process = subprocess.Popen(stockfish_path, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    def get_perft_result(fen):
        stockfish_process.stdin.write(f"position fen {fen}\n")
        stockfish_process.stdin.write(f"go perft {depth}\n")
        stockfish_process.stdin.flush()
        perft_result = None
        while True:
            output = stockfish_process.stdout.readline()
            if output.startswith("Nodes searched: "):
                perft_result = output.split(":")[1].strip()
                break
        return perft_result
    with open(sys.argv[2], 'r') as f:
        fens = f.read().splitlines()
    with open(sys.argv[3], 'w') as f:
        for fen in tqdm(fens):
            perft_result = get_perft_result(fen)
            f.write(f"{fen};{depth};{perft_result}\n")


if __name__ == "__main__":
    main()
