# Test checklist
Test workflows to be done before any release.

- Start application -> No crash.
- Enter ARR and Storage account credentials -> The status changes to "connected"

## Upload panel

- Open Upload panel -> it shows the blob containers for the configured storage account.
### Uploading
- Drag and drop an fbx file -> It shows the new file current directory.
- Select the file and press "del" -> The file is deleted.
- Drag-drop and remove a whole directory -> It shows the directory and removes the directory.
- Drag-drop and remove multiple files and a directory at the same time
- Drag-drop multiple files and a directory at the same time
- Try all of the above with "select file" button.
- During multi-file upload, check that there is a "progress bar" on top of the main Upload button, showing the progress.
### Directories
- Make sure you have multiple directories. Upload some, if you don't.
- Double click a subdirectory -> The current directory is set to the subdirectory.
- From a directory, double click '../' in the list -> Navigates to the parent directory.
- Bread crumbs above the blob list should always show the current directory path.
- Click on a parent "crumb" button -> The current directory is set to the clicked one.
- Click '>' on the root crumb -> The subdirectories should appear in a drop-down list.
- Click on any subdirectory in the drop-down list -> It navigates to the subdirectory.
- Click on the "Add sub-directory" button on the right of the breadcrumbs -> A text-field appears and it has focus
- Press Esc or click on another control -> The text-field disappears
- Type a name and press enter -> The current directory is set to the "new directory"
- Navigate to the parent directory -> The new directory shouldn't show up in the list
- Enter a new directory again. This time upload a blob and go to the parent dir -> The new directory shows in the list
- Enter an existing sub-directory -> It navigates to that directory
- Enter an existing sub-path like "Dir1/Dir2" -> It navigates to the sub-path

## Conversion panel

- Open Conversion panel clicking "Convert"

### Manage conversions

- Press "New" -> A new conversion called "conversion 1" is created and selected.
- Press "New" multiple times -> Multiple conversions are created with a unique name "Conversion X".
- Select a conversion and press the "Delete" button -> The Conversion entry is deleted.
- Select a conversion and type a new name in the textbox -> The conversion immediately changes name.
- Check that "Start conversion" button on the bottom right is not enabled for a new conversion.

### Input selection

- The "Input 3d Model" field show read "[Select input model]". Click on "Select" besides it -> A blob explorer panel appears.
- Select a container with models -> Verify that its blobs are visualized.
- Click on "Show all models"
  - Verify that all and only the glb/gltf/fbx models are visualized as a flat list.
  - Verify that the list entries show the model name and under it the full path.
  - Navigate in subdirectories using the bread crumbs -> The list is filtered to the current directory and subdirectories.
  - Click on 3D model files and other files in the list -> The "Ok" button is only enabled for 3D model files
- Verify that drag-drop and "upload files" behaves exactly like the Upload panel.
- Double click a model file -> It goes back to the conversion panel where the "input 3d model" is set to that model.
- Click "Select" again, this time just select another model and press "OK" -> The input model is selected
- Click "Select" again, select a model but press "Cancel" -> The input model remains unchanged

### Output selection

- The "Output" field should read "[Select output directory]. Click on "Select" besides it. -> A blob explorer panel appears
- When "Show all" is not pressed -> The list contains just the subdirectories from the current directory.
- When "Show all" is pressed -> All of the files in the current directory (and the subdirectories) are shown.
- Enter a directory and press "OK" without selecting anything -> The current subdirectory is selected in the "Output" field
- Click "Select" again. Select a sub-directory in the list and press "OK" -> The selected sub-directory is selected as output.
- Click on "Add Sub-directory" button besides the bread-crumbs, enter a subdirectory and press "OK" -> The new subdirectory is selected as output.

### Running conversion status

- Check that the "Start conversion" button is enabled when input model and output directory are selected.
- Select a small input model and click on "Start conversion"
  - The status field in the panel should read "Converting".
  - The icon besides the conversion in the conversion list should change to a "play" icon.
  - The time on the right of the conversion in the list should start incrementing.
  - The "Convert" button on top should show a loading animation and a small "play" icon with a counter, showing the number of running conversions.
- Wait for the conversion to end
  - If it succeeds the icon and the status would reflect completed/failed status.
  - The time on the conversion should stop running and will indicate the total conversion time.
  - When no conversion are running anymore -> The loading animation should stop, and the conversion counter should disappear.
- Run multiple conversions at the same time -> The conversion counter should shows a number > 1
- While a conversion is running, click the main "Upload" button, to hide the conversion panel. Wait for the conversion to finish
  - The small icon would have a counter of the succeeded or failed conversions.
  - Click on the Convert button again -> The succeeded/failed counter should disappear
- Start a conversion without specifying a name -> The conversion should take the name from the input model
- After the conversion completes click on "Start conversion" again -> It should re-start the conversion

### Conversion configuration

- Verify that the fields can be changed.
- Click on "Reset" -> The fields are all reset to their default values.
- Verify that the configuration is preserved for each input directory:
  - Switch between two input directories and change configuration for each -> values are preserved
  - Switch between two different conversions with two different directories or containers -> values are preserved
- Choose an input model in a path under two or more subdirectories (dir1/dir2/dir3/model.fbx) -> "Input root directory" combo-box will show right under the "Input 3D Model" field in the conversion panel.
  - Verify that the drop-down list shows the directory of the model and all of its parents ("dir1/dir2/dir3"; "dir1/dir2/"; "dir1/"; "[root]")
  - Click on one of the directories -> verify that the configuration doesn't change
- Change some of the default configuration and start the conversion.
- Open the Azure Storage Explorer and inspect the input blob container
- Navigate to the input directory
- Verify that a file "conversionSettings.json" is created besides the model file (also when the input root directory is set to a parent directory)
- Verify that the file contains all of the values that were set in ARRT
  
## Rendering panel

### Session

- Click on the button "Render".
- Verify that the controls are editable.
- Select Max Time "00:10" and Automatic Extension as "00:10" (Auto make sure "Auto Extend" is pressed).
- Start session.
- Verify that the Max Time and Extension Time are the ones you specified.
- Check that the button now shows "Stop Session".
- Click on "Extend" -> The specified Extend Time is added to both "Max Time" and "Remaining Time".
- Click on "Stop Session" -> The session is stopped and the panel returns to the previous state.
- Start a session with a very short Max Time. Make sure Auto Extend is active.
- While the session is running verify that:
  - The session status, shown on the top of the session panel, says "Connected" and the time shown is the remaining time.
  - The "Render" button shows the status icon and has the session information (status/remaining time) in the tooltip.
  - From the session panel, "inspect session" is enabled.
  - Click in "inspect session" -> ArrInspector starts and shows live data from the ARR session (you will see the logs).
  - The panel behind the session panel (model selection panel) becomes active and allows you to select a model.
- Wait for the session to be near expiration time -> The session time will automatically extend.

### Model selection panel

- Make sure the session is connected and the panel is enabled.
- Make sure the "input mode" is set to "From storage container".
- Verify that the blob explorer for the model selection is working (it behaves like the model input).
- Select a model to load and click load -> A progress bar appears and will load the model.
- Once it's loaded press "unload" on the top -> Return to the loading page.
- Double click on a model -> It loads the model.
- Select "From SAS URL" as input mode -> See a text box to enter the URL.
- Try builtin://Engine -> The engine demo model is loaded
- Unload, generate a Sas URI from Azure Storage Explorer to a converted asset (.arrAsset) and input it in the field -> The model is loaded
- Unload the model
- From the main settings, disconnect to the Storage Account, by changing for example the name, and pressing "Retry"
- Check that the only available option now is "From Sas URL" and check if it still works.
- Check also that no conversion or upload operation is available
- Restore the Storage Account settings and load a model

### Viewport

- Check the loaded model is rendering.
- Press WASD, arrow keys -> Camera moves.
- Press right mouse button and drag, 2468, ZXFC, Ins Home Del End -> Camera rotates.
- Press QE, -+, PgUp PgDown -> Camera moves vertically.
- Press shift while moving the camera -> Camera moves 10 times faster.
- Open the settings panel and change the camera settings -> Observe that the changes are immediately applied to the viewport and the camera movements.
- Change the Video Settings (resolution and refresh rate) -> The "Apply" and "Reset" buttons are enabled
- Press "Reset" -> the previous values are restored and the "Apply" and "Reset" buttons are disabled
- Change the Video Settings again and press "Apply" -> The model is unloaded and the session is reconnected.
- Verify that the video settings are applied (maybe change the resolution to a very low value)

### Selection and Scene tree

- Verify that on the left side the panel (scene tree) shows a tree with all of the entities in the loaded model
- Click on the expand arrow on the left -> The entity shows its child entities
- Click on the model in the viewport. Verify that:
  - The clicked part under the mouse, is highlighted.
  - The same entity is highlighted in the scene tree.
  - If the entity was in a collapsed part of the scene tree, the tree is expanded, and scrolled to show the selection.
  - The material list shows only the materials in the selected entity.
- Click on an empty part of the viewport. Verify that:
  - The selection is removed from the viewport.
  - The selection is removed from the scene tree.
  - The material list shows all of the materials in the scene.
- Select multiple entities from the scene tree by holding shift (range select) or control (toggle single selection).
  - The material list shows the union of the the materials in the selected entities. 
- Select an entity from the viewport.
- Select one of the materials from the material list.
- Select the root node from the scene tree -> The selected material show stay selected, in the larger material list.
- Modify the values from the material property editor on the right (for example the albedo color) -> observe the changes in the viewport immediately.

## Logs 

- Open the log panel -> The logs are shown on the bottom as a list on top of another panel.
- Select one of the log entries -> The full text is displayed in the panel below.
- Change the filters on the top -> The log list only shows the type of messages selected.
- Switch to "detailed log view" (top right) -> The list shows column headers, and two extra columns "Date" and "Location"
- Activate the "Debug" type and hide the logs view.
- Do some operation (like starting a session, modifying the settings etc) -> On the "Logs" button there is a counter of the new log messages and the highest severity (affected by the active log filter).
- Open the log view -> The new message counter is reset.

## Configuration persistence

- After configuring, close ARRT and restart it -> The configuration is preserved.
- Start a session, close ARRT and restart it -> The session is reconnected automatically.
- After selecting an input for conversion, create a new conversion and select another input -> The blob browser opens on the previous container.
- Close and restart ARRT -> The blob browser opens on the previous container.