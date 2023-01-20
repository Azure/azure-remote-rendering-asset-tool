# Test checklist

Test workflows to be done before any release.

- Start application -> No crash.

## Settings & Connection

If you've never used ARRT on this PC before, the settings dialog opens automatically, otherwise *Settings > Account Settings...*

- Enter ARR and Storage account credentials
- Use `Test Connection` to check -> message box should show whether it worked
- Connection status should be shown in the status bar

## Help menu

- Check that all entries in the *Help* menu open correctly
- Check that *Help > About ARRT* shows the new version number

## Storage tab

### Containers

- Press '+' to add a new container -> the file list only shows the root folder
- Switch containers, check that the file lists update as expected
- Delete a container (trash can icon)
- The 'Refresh' button should only make a difference while multi-file uploads are ongoing, or if something is changed with another tool in parallel (like Azure Storage Explorer)

## File structure

- Click the 'Add folder' button and create some folder structure -> these empty folders will remain, even between runs
- Also works with CTRL+SHIFT+N
- Click the 'Delete item' button, check that it would delete the selected file or folder
- Also works with 'del'
- Check that the tree view works as you'd expect

### Uploading

- Click 'Upload files' and upload multiple files -> should show a file counter in the status bar
- Click 'Upload folder' and upload an entire folder -> should show a file counter in the status bar
- After all file uploads are finished, the main window will refresh (this may collapse changed folders)

## Conversion tab

### Start conversion

- Select '\<new conversion\>' entry -> edit it on the right
- Click 'Select Asset...' and pick a source asset (only FBX, GLB, GLTF should be shown)
- The 'Name' field displays the filename as the default/fallback name -> can be overridden
- The 'Input folder' field displays the file's parent path as the default input folder.
- Click 'Select Output Folder...' and pick a target destination (only folders should be shown)
- Now the 'Start Conversion' button should be enabled.
- Click 'Select Input Folder...' -> you should only be able to select any parent folder, but nothing else
- Click 'Show advanced options' and check that it behaves as expected
  - e.g. 'Reset to Defaults' should reset all advanced options
- Click 'Start Conversion' -> conversion starts, a new '\<new conversion\>' entry appears on the left
- The running conversion entry shows a *play* icon and the running time
- The status bar shows that 1 conversion is running
- The running conversion is not editable anymore

### Conversion status

- Wait for the conversion to end
- If it succeeds the icon and the status would reflect completed/failed status.
- The time on the conversion should stop running and will indicate the total conversion time.
- On success/failure a label below the advanced options should display the result ("finished successfully" or an error message)
- When no conversion are running anymore -> the conversion counter should disappear from the status bar

### Conversion options

- Run another conversion, with some non-default advanced options
- Open the Azure Storage Explorer and navigate to the **input** asset -> there should be a ".ConversionSettings.json" file next to it
- Check that all options in it are as expected

## Rendering tab

### Session

- Click 'Session...'
- Set *Max Time* to "0:05" and *Extend By* to "0:01" and enable *Auto Extend*
- Start session.
- Wait for the session to start
- Open 'Sessions...' again
- Check that the *Max Time* and *Remaining Time* are the ones you specified
- Check that now the *Stop* button is enabled
- Click on *Extend Now* -> The specified time is added to both *Max Time* and *Remaining Time*
- Wait a bit (or use a session with a shorter max time) and check the log to see that the auto-extension kicks in
- Click the *Open ArrInspector* button and check that it's working

### Models / Rendering / Camera

- With a running session, test the *Load Model from Storage* and *Load Model with URL* buttons
- There should be a progress bar at the bottom when any model loading is ongoing
- Verify that the first loaded model gets centered in the viewport
- You can add multiple models, use *Remove Models* to clear them all
- Test that the *Model Scale* option works
- Use the *Camera...* button to adjust camera options (near/far plane, camera speed)
- In the settings, disconnect from the storage account (e.g. by changing the name to be invalid)
- Check that the only available option now is "Load Model with URL"
- Check also that no conversion or upload operation is available
- Open the *Camera* dialog, there are instructions how to use the camera, check that they all work

### Selection and Scene tree

- Verify the scene tree shows every model and its children (there may be no children, if the object is converted that way)
- Click on the model in the viewport. Verify that:
  - The clicked part under the mouse, is highlighted.
  - The same entity is highlighted in the scene tree.
  - The material list shows only the materials of the selected entity (and children)
- Click on an empty part of the viewport. Verify that:
  - The selection is removed from the viewport.
  - The selection is removed from the scene tree.
  - The material list is now empty
- Double click on various entities in the scene tree -> the entity is centered and framed in the viewport
- Select multiple entities from the scene tree by holding shift (range select) or control (toggle single selection).
  - The material list shows the union of the the materials in the selected entities.

## Material editing

- Select an entity
- Select one of the materials from the material list
- Modify the values from the material property editor on the right (for example the albedo color) -> observe the changes in the viewport immediately

## Log tab

- Upload some files -> should be shown here
- Start / stop a session -> should show up
- Use session auto extension -> should show up here
- After doing lots of stuff it should NOT be spammed full of messages
- The tab's icon should show the highest type of log message (debug/info/warning/error).

## Configuration persistence

- After configuring, close ARRT and restart it -> the configuration is preserved
- The settings in the 'Session' dialog should persist
