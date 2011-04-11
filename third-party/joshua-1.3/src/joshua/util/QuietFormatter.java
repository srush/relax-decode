/* This file is part of the Joshua Machine Translation System.
 * 
 * Joshua is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
package joshua.util;

import java.util.logging.Formatter;
import java.util.logging.LogRecord;

/**
 * Log formatter that prints just the message, with no time stamp.
 *
 * @author Lane Schwartz
 * @version $LastChangedDate: 2009-05-22 23:31:12 -0500 (Fri, 22 May 2009) $
 */
public class QuietFormatter extends Formatter {

    public String format(LogRecord record) {
        return "" + formatMessage(record)+ "\n";
    }

}
