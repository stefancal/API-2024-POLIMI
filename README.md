# Pastry Shop Order Management Simulator

**Final project for Algorithms and Data Structures 2023-2024**

This project simulates the operation of an industrial pastry shop, specifically focusing on order management, inventory handling, and logistics in a discrete-time simulation environment. The simulation is entirely command-driven and processes events such as recipe management, ingredient restocking, order processing, and courier pickups.

<p align="center">
  <img src="img/final grade.png" alt="Final Grade Screenshot" width="400"/>
  <br>
  <em>Final grade for the project</em>
</p>

## Features

- **Discrete Time Simulation:** Each command advances the simulation time by one unit.
- **Ingredients & Recipes:** Manage a catalog of recipes, each requiring various ingredient quantities.
- **Inventory System:** Track ingredient batches with expiry dates and quantities.
- **Order Handling:** Accept and process multi-item orders, handling insufficient stock with a pending order queue.
- **Prioritized Ingredient Consumption:** Ingredients are always used from batches with the earliest expiry.
- **Courier Logistics:** Periodically ships ready orders, maximizing load while respecting weight limits and order chronology.
- **Command-Line Interaction:** All simulation actions are performed via well-defined commands.

## Problem Statement

An industrial pastry shop wishes to improve its order management system by developing software that simulates its operations. The simulation manages:

- **Recipes:** Defined by a name and a list of required ingredients (by name and amount in grams).
- **Ingredients:** Each ingredient is tracked in the warehouse with quantity and expiry (time-based).
- **Orders:** Customers place orders for one or more pastries, which are prepared if ingredients are available.
- **Pending Orders:** If not enough ingredients are present, orders are queued until stock is replenished.
- **Courier Collection:** At fixed intervals, a courier collects orders, loading them by chronological order and maximizing the truck's capacity.

## Input Format

The input text file consists of:

1. **First Line:** Two integers — courier frequency (in time units) and truck capacity (in grams).
2. **Subsequent Lines:** A sequence of commands, one per line. Supported commands include:
   - `aggiungi_ricetta <recipe_name> <ingredient_name> <quantity> ...`
   - `rimuovi_ricetta <recipe_name>`
   - `rifornimento <ingredient_name> <quantity> <expiry> ...`
   - `ordine <recipe_name> <number_of_items>`



### Command Details

- **aggiungi_ricetta:** Adds a new recipe with its required ingredients. Ignores duplicates.
- **rimuovi_ricetta:** Removes a recipe unless there are pending orders for it.
- **rifornimento:** Restocks the warehouse with batches of ingredients, each with a given quantity and expiry.
- **ordine:** Places an order for a given number of items using a specified recipe if available.

**At each courier interval**, the program prints the truck's load as a sequence of `<order_arrival_time> <recipe_name> <number_of_items>` entries, sorted by loading order. If the truck is empty, it prints "camioncino vuoto".

## Example Input

```
10 5000
aggiungi_ricetta meringhe_della_prozia zucchero 100 albumi 100
rifornimento zucchero 200 150 albumi 200 150
ordine meringhe_della_prozia 5
```

## Example Output

```
aggiunta
rifornito
accettato
camioncino vuoto
```

## Usage

1. **Compile** the source code (language-specific instructions here, e.g. `gcc main.c -o pastry_simulator`).
2. **Run** the simulator with the input file provided (open11):
   ```
   ./pastry_simulator < open11.txt
   ```
3. **Interpret the output** as described above.

## Technologies Used

- Language: C
- Standard libraries only required
