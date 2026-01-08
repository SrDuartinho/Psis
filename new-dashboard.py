import time 
import os

def main():
    """
        Main function for the new dashboard.
        Expects a file named 'dashboard.txt' to be present.
        File should have the following format:
        Lines 1-n: Planets data -> A - 0, B - 3, ...
        Lines n+1: "----"
        Lines n+2-m: Spaceships data -> X - 5, Y - 2, ...
        Line m+1: "****"
        Line m+2: Total trash count
        Line m+3: Maximum trash capacity
    """
    planets = []
    spaceships = []
    trash_data = []

    while True:
        if not os.path.exists("dashboard.txt"):
            print("Data file 'dashboard.txt' not found. Waiting...")
            time.sleep(2)
            continue
        with open("dashboard.txt", "r") as f:
            planets_read = False
            spaceship_read = False
            for line in f:
                line = line.strip() # Remove whitespace/newline characters
                if line == "----":  # Separator between planets and spaceships data
                    planets_read = True
                    continue
                if line == "****":
                    spaceship_read = True
                    continue
                if not planets_read:
                    planets.append(line)
                elif not spaceship_read:
                    spaceships.append(line)
                else:
                    trash_data = line.split()
                    total_trash = trash_data[0]
                    max_trash = trash_data[1]

        print("\033c", end="")  # Clear console
        print("Planets (Recycled trash):")
        for planet in planets:
            print("   " + planet)
        print("Spaceships (Trash cargo):")
        for spaceship in spaceships:
            print("   " + spaceship)
        print(f"Roaming trash: {total_trash}")
        print(f"Trash capacity: {int(total_trash)/int(max_trash)*100:.2f}%")
        planets.clear()
        spaceships.clear()
        trash_data.clear()
    
        time.sleep(2)
        
    
if __name__ == "__main__":
    main()