# Bike racing physics: papers and sources (researched 2026-10-13)

User asked: "is there papers on bike racing game physics?" No game-specific papers or GDC talks found; the field is vehicle-dynamics engineering.

## Most useful for a game
- Limebeer & Sharp, "Bicycles, motorcycles, and models: single-track vehicle modeling and control", IEEE Control Systems Magazine 26(5):34-61, Oct 2006, doi 10.1109/MCS.2006.1700044. Readable review. https://spiral.imperial.ac.uk/entities/publication/8ffa665c-c948-4f92-9dc9-3e4467ae3762
- Cossalter & Lot, "A motorcycle multi-body model for real time simulations based on the natural coordinates approach", Vehicle System Dynamics 37:423-447, 2002. 11 DOF, own tyre model with carcass elasticity / relaxation; runs real time in FastBike. https://www.research.unipd.it/handle/11577/2473624 , https://www.dynamotion.it/wp-content/uploads/2018/07/fastbike_ref.pdf
- Meccanica 2022 (s11012-022-01603-8): low-complexity riding simulator = tuned understeering single-track car model, high yaw inertia, steer torque converted to car steer angle; cheapest model with motorcycle-like response. https://link.springer.com/10.1007/s11012-022-01603-8
- Mechanical Engineering Journal, Nov 2024 (Hatakeyama et al.): real-time riding model with NMPC rider, steer torque input, reproduces weave/wobble. https://doaj.org/article/605f26eb660e412aafde9cccf6331dd4
- Pless 2023 TU Darmstadt thesis: rider lean as an input (rider-induced roll torque). https://tuprints.ulb.tu-darmstadt.de/23983
- KTH motorcycle simulator report: https://www.kth.se/social/files/605dfdafc2a776ae95e474ca/mcsim.pdf

## Classics
- Sharp 1971, "The Stability and Control of Motorcycles", J. Mech. Eng. Sci. 13(5):316-329. https://trid.trb.org/View/34815
- Meijaard, Papadopoulos, Ruina, Schwab 2007, "Linearized dynamics equations for the balance and steer of a bicycle: a benchmark and review", Proc. R. Soc. A 463:1955-1982, doi 10.1098/rspa.2007.1857. Whipple benchmark. PDF: https://eklausmeier.lima-city.de/pdf/meijaard2007.pdf ; parameters tool: https://bicycleparameters.readthedocs.io/en/latest/description.html
- Kooijman et al. 2011, "A Bicycle Can Be Self-Stable Without Gyroscopic or Caster Effects", Science 332:339-342, doi 10.1126/science.1201959. PDF: https://gwern.net/doc/technology/2011-koojiman.pdf

## Books (not searched)
- V. Cossalter, Motorcycle Dynamics. H. Pacejka, Tire and Vehicle Dynamics (magic formula, motorcycle tyre chapter).

## Games built on this (no papers, but descriptions)
- GP Bikes (own engine, industry tyre model, chassis flex): https://gp-bikes.com/
- TrackDayR (Pacejka MF tyres, textbook dynamics): https://rawg.io/games/trackdayr
- RIDE forum anecdote: countersteer prototypes felt wrong, dropped for direct lean: https://steamcommunity.com/app/345660/discussions/0/618457398975203328
