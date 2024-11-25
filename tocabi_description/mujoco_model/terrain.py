import random

# Grid parameters
x_range = range(1, 6)  # x from -1 to 5
y_range = range(0, 1)  # y from -3 to 2
size = "0.01 1 0.016"
rgba_1 = "0.3 0.3 0.3 1"
rgba_2 = "0.6 0.6 0.6 1"  # Alternate color for random squares
scatter_range = 0.8  # Scatter range for random offset

# Initialize an empty string to hold the XML structure
xml_output = "<!-- Scattered Random Square Pattern -->\n"

# Generate the XML structure with random placement, colors, and scatter
for y in y_range:
    for x in x_range:
        # Randomly decide to place a square or leave the space empty
        if random.choice([True, False]):
            rgba = random.choice([rgba_1, rgba_2])  # Randomly select one of the colors
            # Apply a random scatter to the x and y positions
            x_scattered = x + random.uniform(-scatter_range, scatter_range)
            y_scattered = y + random.uniform(-scatter_range, scatter_range)
            # Add the box geometry with scattered position
            xml_output += f'    <geom type="box" size="{size}" pos="{x_scattered:.2f} {y_scattered:.2f} 0.00" rgba="{rgba}"/>\n'

# Save the generated XML structure to a file
with open("scattered_random_square_pattern.xml", "w") as file:
    file.write(xml_output.strip())
    
# Generate the XML structure with random placement, colors, and scatter
for y in y_range:
    for x in x_range:
        # Randomly decide to place a square or leave the space empty
        if random.choice([True, False]):
            rgba = random.choice([rgba_1, rgba_2])  # Randomly select one of the colors
            # Apply a random scatter to the x and y positions
            x_scattered = x + random.uniform(-scatter_range, scatter_range)
            y_scattered = y + random.uniform(-scatter_range, scatter_range)
            # Add the box geometry with scattered position
            xml_output += f'    <geom type="box" size="{size}" pos="{x_scattered:.2f} {y_scattered:.2f} 0.00" rgba="{rgba}"/>\n'

# Save the generated XML structure to a file
with open("scattered_random_square_pattern.xml", "w") as file:
    file.write(xml_output.strip())
    
# Generate the XML structure with random placement, colors, and scatter
for y in y_range:
    for x in x_range:
        # Randomly decide to place a square or leave the space empty
        if random.choice([True, False]):
            rgba = random.choice([rgba_1, rgba_2])  # Randomly select one of the colors
            # Apply a random scatter to the x and y positions
            x_scattered = x + random.uniform(-scatter_range, scatter_range)
            y_scattered = y + random.uniform(-scatter_range, scatter_range)
            # Add the box geometry with scattered position
            xml_output += f'    <geom type="box" size="{size}" pos="{x_scattered:.2f} {y_scattered:.2f} 0.00" rgba="{rgba}"/>\n'

# Save the generated XML structure to a file
with open("scattered_random_square_pattern.xml", "w") as file:
    file.write(xml_output.strip())

print("XML code for scattered random square pattern has been saved to 'scattered_random_square_pattern.xml'")

#?
# import random

# # Parameters for the grid and cylinder dimensions
# x_start, x_end = 1, 6
# y_start, y_end = -2, 2
# spacing = 0.5  # Adjust spacing between cylinders
# cylinder_length = 0.3  # Length along the x-axis
# cylinder_radius = 0.01  # Radius of the circular face
# scatter_range = 0.8  # Scatter range for random offset

# # Base XML template for a cylinder with circular face along x-axis
# cylinder_template = '<geom type="cylinder" size="{radius} {length}" pos="{x:.3f} {y:.3f} 0" quat="0 0.707 0 0.707" rgba="{r} {g} {b} 1"/>\n'

# # Function to generate the XML for a grid of cylinders with scattering
# def generate_cylinder_grid_with_scatter(x_start, x_end, y_start, y_end, spacing, radius, length, scatter_range):
#     xml_output = ""
#     z_pos = radius  # Position above ground based on radius

#     for x in range(x_start, x_end + 3):
#         for y in range(y_start, y_end + 3):
#             # Calculate actual positions in the grid with specified spacing
#             x_pos = x * spacing + random.uniform(-scatter_range, scatter_range)
#             y_pos = y * spacing + random.uniform(-scatter_range, scatter_range)
#             # Optional color pattern: alternate colors based on grid position
#             r, g, b = (0.6, 0.6, 0.6) if (x + y) % 2 == 0 else (0.3, 0.3, 0.3)
            
#             # Format cylinder XML with scattered positions and color
#             xml_output += cylinder_template.format(
#                 radius=radius,
#                 length=length,
#                 x=x_pos,
#                 y=y_pos,
#                 z=z_pos,
#                 r=r,
#                 g=g,
#                 b=b
#             )
#     for x in range(x_start, x_end + 3):
#         for y in range(y_start, y_end + 3):
#             # Calculate actual positions in the grid with specified spacing
#             x_pos = x * spacing + random.uniform(-scatter_range, scatter_range)
#             y_pos = y * spacing + random.uniform(-scatter_range, scatter_range)
#             # Optional color pattern: alternate colors based on grid position
#             r, g, b = (0.6, 0.6, 0.6) if (x + y) % 2 == 0 else (0.3, 0.3, 0.3)
            
#             # Format cylinder XML with scattered positions and color
#             xml_output += cylinder_template.format(
#                 radius=radius,
#                 length=length,
#                 x=x_pos,
#                 y=y_pos,
#                 z=z_pos,
#                 r=r,
#                 g=g,
#                 b=b
#             )
    
#     return xml_output

# # Generate the XML code with random scattering
# xml_code = generate_cylinder_grid_with_scatter(x_start, x_end, y_start, y_end, spacing, cylinder_radius, cylinder_length, scatter_range)

# # Write the XML code to a text file without additional line spaces
# with open("scattered_cylinder_grid.txt", "w") as file:
#     file.write(xml_code.strip())  # .strip() removes any leading/trailing whitespace

# print("XML code for scattered cylinders has been written to 'scattered_cylinder_grid.txt'")


# #?

# import random

# # Parameters for the scattered spheres
# x_range = (1, 6)  # Range for x-coordinate
# y_range = (-2, 2)  # Range for y-coordinate
# z_pos = 0.0        # Fixed height for all spheres
# num_spheres = 400     # Number of spheres to scatter
# sphere_radius = 0.01  # Radius of each sphere

# # Base XML template for a sphere
# sphere_template = '<geom type="sphere" size="{radius}" pos="{x} {y} {z}" rgba="{r} {g} {b} 1"/>\n'

# # Function to generate XML for scattered spheres
# def generate_scattered_spheres(x_range, y_range, z, num_spheres, radius):
#     xml_output = ""

#     for _ in range(num_spheres):
#         # Generate random x, y coordinates within the specified ranges
#         x = random.uniform(*x_range)
#         y = random.uniform(*y_range)
        
#         # Random color for each sphere
#         r, g, b = random.random(), random.random(), random.random()
        
#         # Format sphere XML with calculated positions and color
#         xml_output += sphere_template.format(
#             radius=radius,
#             x=x,
#             y=y,
#             z=z,
#             r=r,
#             g=g,
#             b=b
#         )
    
#     return xml_output

# # Generate the XML code
# xml_code = generate_scattered_spheres(x_range, y_range, z_pos, num_spheres, sphere_radius)

# # Write the XML code to a text file
# with open("scattered_spheres.xml", "w") as file:
#     file.write(xml_code.strip())

# print("XML code for scattered spheres has been written to 'scattered_spheres.xml'")
