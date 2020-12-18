import sys

def main():
	print("ready")
	
	while(True):
		line = input()
		if len(line) > 0:
			if line == "shutdown":
				return
			else:
				print("10")
	
if __name__ == "__main__":
	main()