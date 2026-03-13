import json
from pathlib import Path

def process_transactions(input_filename, output_filename):
    # 1. Load the data
    input_path = Path(input_filename)
    
    if not input_path.exists():
        print(f"Error: The file '{input_filename}' was not found.")
        return

    with open(input_path, 'r') as f:
        data = json.load(f)

    # 2. Iterate and convert types
    count = 0
    for txn_id, details in data.items():
        if "amount" in details:
            # Convert to float (handles ints and numeric strings)
            details["amount"] = float(details["amount"])
            count += 1

    # 3. Save the updated data
    with open(output_filename, 'w') as f:
        json.dump(data, f, indent=2)
    
    print(f"Success! Processed {count} transactions.")
    print(f"Updated file saved as: {output_filename}")

# Run the function
# Replace 'data.json' with your actual filename
process_transactions('resources/data/transactions.json', 'resources/data/transactions_clean.json')