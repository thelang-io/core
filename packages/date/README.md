# the/date
Date library

## Installation

```bash
the install the/date
```

## Example usage

```the
import * as date from "the/date"
```

## API

### `format (datetime: u64) str`
Converts a Unix timestamp to a formatted date-time string.

**Parameters**

- `datetime` - Unix timestamp in seconds

**Return value**

A formatted string representing the date and time in "YYYY-MM-DD HH:mm:ss" format.

**Examples**

```the
formattedDate := date.format(1709251200)
```
