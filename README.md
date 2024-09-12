
<h1> City Simulation Brief report</h1>
<h2 align="center">Hicham Kitaz</h2> 
<h2 align="center">Abd Alwahed Haj Omar</h2> 
<h3>Introduction</h3>
<p>Imagine a city that breathes and grows as you explore – a digital realm where streets form and
buildings rise with each step. Our project harnessed OpenGL's power and procedural generation's
magic to create this living cityscape. Walk through, and watch textures shift in response to changing
light, adding a touch of realism to the experience.
This report dives into the fusion of creativity and technology that birthed this dynamic world,
showing the potential of interactive simulations. Welcome to a city that evolves with you.</p>
<p align="center">
  <img src="https://github.com/Abdoul111/openGL_COMP371_PROJECT/assets/111257875/bf103f19-dbd9-452b-b24c-1a053478a986">
</p>
<h3>Demo</h3>
<h4>Video Overview</h4>
<a href="https://youtu.be/zP9QrJyelwk">
  <img align="center" src="https://github.com/Abdoul111/openGL_COMP371_PROJECT/assets/111257875/35eb3096-b164-47a7-b353-27c9779e7fbd" alt="image">
</a>


<h3>Roles</h3>
<p><strong>Abdelwahed:</strong> Redefined procedural generation by crafting an algorithm that uses advanced
object-oriented programming and data structures. As you move through the world, a dynamic,
entirely randomized environment materializes. This isn't just randomness – it's a harmonious
interplay of carefully designed data structures that create a world that's responsive, unique, and alive.
With Abdelwahed's innovation, exploration takes on a whole new dimension, where every step
reveals a world crafted by algorithms that adapt and evolve with you.</p>

<p><strong>Hicham:</strong> he is the driving force behind our project's success, seamlessly blending design finesse with
coding expertise. He meticulously enhanced textures, modeled the game world and contributed
innovative algorithmic solutions that propelled our project to new heights. Hisham's meticulous
testing unearthed and rectified elusive bugs, ensuring a flawless user experience. Beyond technical
contributions, his motivational spirit, organizational skills, and insights into refining key
functionalities were instrumental in keeping the team inspired and the project on track. Hisham's
multidimensional role was indispensable in turning our vision into a reality.</p>

<h3>Algorithm</h3>
<p>This project went into multiple phases that are described below:</p>

<p><strong>Modelling and texturing:</strong> One of the most time-consuming phases, involved crafting the very
essence of our virtual world. Our team invested substantial effort in meticulously shaping each
element. We paid meticulous attention to detail as we designed and added textures to breathe life into
our creation. The cornerstone of our world became the 'square' – a fundamental building block that
set the stage for our entire universe. Through careful coding, we constructed functions that rendered these squares and laid the groundwork for scalability. This strategic approach allowed
us to seamlessly replicate and call these functions for diverse locations, a critical functionality for
generating new squares in subsequent stages.</p>

<p><strong>Lights, shadows, spotlight, camera, key calls:</strong> we harnessed these essential elements to craft a
captivating virtual world that seamlessly evolves as players explore.
Auto-generating the world while walking: Our team devised a clever mechanism within the
keyCall function that constantly monitors the player's movements. With each step taken, the
function would intelligently check if the player transitioned from one square to another. This pivotal
check ensured that we seamlessly triggered the square generation functions for the new location,
thereby expanding the world organically.</p>

<p><strong>Making the world evolve randomly:</strong> Diversity and unpredictability were at the heart of our
approach to crafting an immersive experience. In our quest to create a truly dynamic virtual world,
we engineered a system that introduced an element of randomness into each square's identity.
Through a sophisticated algorithm, our team ensured that as players ventured from one square to
another, they'd encounter varied surroundings. The choice between a towering skyscraper, a cozy
shopfront, a serene fountain, or even a bustling soccer field was made randomly. To enhance the
sense of uniqueness, textures, colors, and experiences were also randomly assigned, ensuring that no
two players encountered the same world. This innovation provided an enriching layer of exploration,
keeping players engaged and curious about what the next square might hold. The culmination of our
collective efforts in design and coding transformed the virtual landscape into a realm of endless
discovery and surprise.</p>

<strong>User Interaction:</strong>

<p>The journey begins at the heart of the city, enveloped by towering trees, skyscrapers, apartment
complexes, and the vibrant ambiance of a simulated football game. An animated fountain graces the
surroundings, its waters in perpetual motion. As the player takes their first step, they embark on a
captivating exploration of the urban landscape.
Strolling through the city streets, the environment unfolds with a touch of randomness. Buildings
materialize in real-time, creating a unique experience every time the game is launched. Abdelwahed's
innovative approach to randomness adds another layer of dynamism, dynamically assigning textures
to buildings, trees, shops, and apartments.</p>

<p>Navigating this digital metropolis is intuitive, guided by a set of responsive keys:</p>
<p align="right">
  <img src="https://github.com/Abdoul111/openGL_COMP371_PROJECT/assets/111257875/cbc5c1a9-f668-4a63-b7ce-f88621d9c68a" style="width: 200px;">
</p>

<ul>
  <li><strong>W:</strong> Move forward</li>
  <li><strong>A:</strong> Move left</li>
  <li><strong>S:</strong> Move backwards</li>
  <li><strong>D:</strong> Move right</li>
  <li><strong>N:</strong> Toggle night mode, illuminating the streets with ambient street lights.</li>
  <li><strong>M:</strong> Activate the flashlight for enhanced visibility.</li>
  <li><strong>1:</strong> Transition to a top-down view.</li>
  <li><strong>2:</strong> Revert to the default player perspective.</li>
  <li><strong>Spacebar:</strong> Toggle shadows on/off, modulating the visual atmosphere.</li>
  <li><strong>Y:</strong> Unlock free flight mode, allowing controlled exploration in three dimensions without intersecting buildings.</li>
</ul>

The core gameplay adheres to the project's cardinal principle: the player's interaction with the urban
realm must be grounded in reality. The camera's journey halts upon contact with buildings,
preventing the player from seamlessly traversing through solid structures. This design element
ensures an immersive experience that aligns with the project's essence, fostering a connection
between the player and the cityscape.

<p><strong>Special feature:</strong> Horror mode.(active only during night mode).
When the player goes too far from the city the lights go out and the player is left without vision, in
order to see the player has to turn on the flashlight. In this mode sometimes buildings are allowed to
disappear making the player think he is hallucinating. Also, some buildings might appear like they are
currently switching to add confusion to the player.</p>
<p align="center">
  <img src="https://github.com/Abdoul111/openGL_COMP371_PROJECT/assets/111257875/0f0b726d-f362-4173-9715-1f055fceefb0">
  <p align="center"><strong>survival mode</strong></p>
</p>
<p align="center">
  <img src="https://github.com/Abdoul111/openGL_COMP371_PROJECT/assets/111257875/c93caef3-ccc1-41f6-8620-12a101d28f3b">
  <p align="center"><strong>Top-down view day</strong></p>
</p>
<p align="center">
  <img src="https://github.com/Abdoul111/openGL_COMP371_PROJECT/assets/111257875/af262104-7c03-4e66-8d2d-f00fd0a4e42c">
  <p align="center"><strong>Top-down view night</strong></p>
</p>
